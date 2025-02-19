/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * Copyright(c) 2008-2021 The hydrogen development team [hydrogen-devel@lists.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see https://www.gnu.org/licenses
 *
 */

#include "SoundLibraryExportDialog.h"

#include <core/Hydrogen.h>
#include <core/Helpers/Filesystem.h>
#include <core/Preferences/Preferences.h>
#include <core/H2Exception.h>
#include <core/Basics/Adsr.h>
#include <core/Basics/Sample.h>
#include <core/Basics/Instrument.h>
#include <core/Basics/InstrumentList.h>
#include <core/Basics/InstrumentLayer.h>
#include <core/Basics/InstrumentComponent.h>
#include <core/Basics/DrumkitComponent.h>

#include <QFileDialog>
#include <QtGui>
#include <QtWidgets>

#include <memory>

#if defined(H2CORE_HAVE_LIBARCHIVE)
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <cstdio>
#endif

using namespace H2Core;

SoundLibraryExportDialog::SoundLibraryExportDialog( QWidget* pParent,  const QString& sSelectedKit, H2Core::Filesystem::Lookup lookup )
	: QDialog( pParent )
	, m_sPreselectedKit( sSelectedKit )
	, m_preselectedKitLookup( lookup )
{
	setupUi( this );
	
	setWindowTitle( tr( "Export Sound Library" ) );
	m_sSysDrumkitSuffix = " (system)";
	updateDrumkitList();
	adjustSize();
	setFixedSize( width(), height() );
	drumkitPathTxt->setText( Preferences::get_instance()->getLastExportDrumkitDirectory() );
}




SoundLibraryExportDialog::~SoundLibraryExportDialog()
{
	INFOLOG( "DESTROY" );

	for (uint i = 0; i < m_pDrumkitInfoList.size(); i++ ) {
		Drumkit* info = m_pDrumkitInfoList[i];
		delete info;
	}
	m_pDrumkitInfoList.clear();
}



void SoundLibraryExportDialog::on_exportBtn_clicked()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	Filesystem::Lookup lookup;
	bool bIsUserDrumkit;
	QString		drumkitName = drumkitList->currentText();
	if ( drumkitName.contains( m_sSysDrumkitSuffix ) ) {
		lookup = Filesystem::Lookup::system;
		bIsUserDrumkit = false;
		drumkitName.replace( m_sSysDrumkitSuffix, "" );
	} else {
		lookup = Filesystem::Lookup::user;
		bIsUserDrumkit = true;
	}
	QString		drumkitDir = Filesystem::drumkit_dir_search( drumkitName, lookup );
	QString		saveDir = drumkitPathTxt->text();
	Drumkit*	pDrumkit = nullptr;
	int nVersionListIndex = versionList->currentIndex();
	
	QDir qdTempFolder( Filesystem::tmp_dir() );
	bool TmpFileCreated = false; 

	int componentID = -1;
	
	if( nVersionListIndex == 1 ) {
		for ( auto pDrumkit : m_pDrumkitInfoList ) {
			if( pDrumkit->get_name().compare( drumkitName ) == 0 &&
				pDrumkit->isUserDrumkit() == bIsUserDrumkit ) {
				QString temporaryDrumkitXML = qdTempFolder.filePath( "drumkit.xml" );
				INFOLOG( "[ExportSoundLibrary]" );
				INFOLOG( "Saving temporary file into: " + temporaryDrumkitXML );
				TmpFileCreated = true; //NOLINT

				for ( auto pComponent : *( pDrumkit->get_components() ) ) {
					if( pComponent->get_name().compare( componentList->currentText() ) == 0) {
						componentID = pComponent->get_id();
						break;
					}
				}
				pDrumkit->save_file( temporaryDrumkitXML, true, componentID );
				break;
			}
		}
		assert( pDrumkit );
	}


#if defined(H2CORE_HAVE_LIBARCHIVE)
	QString fullDir = drumkitDir + "/" + drumkitName;
	QDir sourceDir(fullDir);

	sourceDir.setFilter(QDir::Files);
	QStringList filesList = sourceDir.entryList();

	QString outname = saveDir + "/" + drumkitName + ".h2drumkit";

	struct archive *a;
	struct archive_entry *entry;
	struct stat st;
	char buff[8192];
	int len;
	FILE *f;

	a = archive_write_new();

	#if ARCHIVE_VERSION_NUMBER < 3000000
		archive_write_set_compression_gzip(a);
	#else
		archive_write_add_filter_gzip(a);
	#endif

	archive_write_set_format_pax_restricted(a);
	int ret = archive_write_open_filename(a, outname.toUtf8().constData());
	if ( ret != ARCHIVE_OK ) {
		QMessageBox::critical( this, "Hydrogen", tr( "Couldn't create archive" )
							   .append( QString( " [%0]" ).arg( outname ) ) );
		return;
	}
	for (int i = 0; i < filesList.size(); i++) {
		QString filename = fullDir + "/" + filesList.at(i);
		QString targetFilename = drumkitName + "/" + filesList.at(i);

		if( nVersionListIndex == 1 ) {
			if( filesList.at(i).compare( QString("drumkit.xml") ) == 0 ) {
				filename = qdTempFolder.filePath( "drumkit.xml" );
			}
			else {
				bool bFoundFileInRightComponent = false;
				for( int j = 0; j < pDrumkit->get_instruments()->size() ; j++){
					InstrumentList instrList = pDrumkit->get_instruments();
					auto instr = instrList[j];
					for ( auto pComponent : *( instr->get_components() ) ) {
						if( pComponent->get_drumkit_componentID() == componentID ){
							for( int n = 0; n < InstrumentComponent::getMaxLayers(); n++ ) {
								auto layer = pComponent->get_layer( n );
								if( layer ) {
									 if( layer->get_sample()->get_filename().compare(filesList.at(i)) == 0 ) {
										 bFoundFileInRightComponent = true;
										 break;
									 }
								}
							}
						}
					}
				}
				if( !bFoundFileInRightComponent ) {
					continue;
				}
			}
		}


		stat(filename.toUtf8().constData(), &st);
		entry = archive_entry_new();
		archive_entry_set_pathname(entry, targetFilename.toUtf8().constData());
		archive_entry_set_size(entry, st.st_size);
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);
		archive_write_header(a, entry);
		f = fopen(filename.toUtf8().constData(), "rb");
		len = fread(buff, sizeof(char), sizeof(buff), f);
		while ( len > 0 ) {
				archive_write_data(a, buff, len);
				len = fread(buff, sizeof(char), sizeof(buff), f);
		}
		fclose(f);
		archive_entry_free(entry);
	}
	archive_write_close(a);

	#if ARCHIVE_VERSION_NUMBER < 3000000
		archive_write_finish(a);
	#else
		archive_write_free(a);
	#endif

	filesList.clear();

	QApplication::restoreOverrideCursor();
	QMessageBox::information( this, "Hydrogen", tr("Drumkit exported.") );
#elif !defined(WIN32)

	if(TmpFileCreated)
	{
		/*
		 * If a temporary drumkit.xml has been created:
		 * 1. move the original drumkit.xml to drumkit_backup.xml
		 * 2. copy the temporary file to drumkitDir/drumkit.xml
		 * 3. export the drumkit
		 * 4. move the drumkit_backup.xml to drumkit.xml
		 */

		int ret = 0;
		
		//1.
		QString cmd = QString( "cd " ) + drumkitDir + "; " + "cp " + drumkitName + "/drumkit.xml " + drumkitName + "/drumkit_097.xml";
		ret = system( cmd.toLocal8Bit() );
		
		
		//2.
		cmd = QString( "cd " ) + drumkitDir + "; " + "mv " + qdTempFolder.filePath( "drumkit.xml" ) + " " + drumkitName + "/drumkit.xml";
		ret = system( cmd.toLocal8Bit() );
		
		//3.
		cmd =  QString( "cd " ) + drumkitDir + ";" + "tar czf \"" + saveDir + "/" + drumkitName + ".h2drumkit\" -- \"" + drumkitName + "\"";
		ret = system( cmd.toLocal8Bit() );

		//4.
		cmd = QString( "cd " ) + drumkitDir + "; " + "mv " + drumkitName + "/drumkit_097.xml " + drumkitName + "/drumkit.xml";
		ret = system( cmd.toLocal8Bit() );

	} else {
		QString cmd =  QString( "cd " ) + drumkitDir + ";" + "tar czf \"" + saveDir + "/" + drumkitName + ".h2drumkit\" -- \"" + drumkitName + "\"";
		int ret = system( cmd.toLocal8Bit() );
	}



	QApplication::restoreOverrideCursor();
	QMessageBox::information( this, "Hydrogen", tr("Drumkit exported.") );
#else
	QApplication::restoreOverrideCursor();
	QMessageBox::information( this, "Hydrogen", tr("Drumkit not exported. Operation not supported.") );
#endif
}

void SoundLibraryExportDialog::on_drumkitPathTxt_textChanged( QString str )
{
	QString path = drumkitPathTxt->text();
	if (path.isEmpty()) {
		exportBtn->setEnabled( false );
	}
	else {
		exportBtn->setEnabled( true );
	}
}

void SoundLibraryExportDialog::on_browseBtn_clicked()
{
	QString sPath = Preferences::get_instance()->getLastExportDrumkitDirectory();
	if ( ! Filesystem::dir_writable( sPath, false ) ){
		sPath = QDir::homePath();
	}

	QString filename = QFileDialog::getExistingDirectory( this, tr("Directory"), sPath );
	if ( filename.isEmpty() ) {
		drumkitPathTxt->setText( sPath );
	} else {
		drumkitPathTxt->setText( filename );
		Preferences::get_instance()->setLastExportDrumkitDirectory( filename );
	}
}

void SoundLibraryExportDialog::on_cancelBtn_clicked()
{
	accept();
}

void SoundLibraryExportDialog::on_drumkitList_currentIndexChanged( QString str )
{
	componentList->clear();

	QStringList p_compoList = m_kit_components[str];

	for (QStringList::iterator it = p_compoList.begin() ; it != p_compoList.end(); ++it) {
		QString p_compoName = *it;

		componentList->addItem( p_compoName );
	}
}

void SoundLibraryExportDialog::on_versionList_currentIndexChanged( int index )
{
	if( index == 0 ) {
		componentList->setEnabled( false );
	} else if( index == 1 ) {
		componentList->setEnabled(  true );
	}
}

void SoundLibraryExportDialog::updateDrumkitList()
{
	INFOLOG( "[updateDrumkitList]" );

	drumkitList->clear();

	for ( auto pDrumkitInfo : m_pDrumkitInfoList ) {
		delete pDrumkitInfo;
	}
	m_pDrumkitInfoList.clear();

	QStringList sysDrumkits = Filesystem::sys_drumkit_list();
	QString sDrumkitName;
	for (int i = 0; i < sysDrumkits.size(); ++i) {
		QString absPath = Filesystem::sys_drumkits_dir() + sysDrumkits.at(i);
		Drumkit *info = Drumkit::load( absPath, false );
		if (info) {
			m_pDrumkitInfoList.push_back( info );
			sDrumkitName = info->get_name() + m_sSysDrumkitSuffix;
			drumkitList->addItem( sDrumkitName );
			QStringList p_components;
			for ( auto pComponent : *(info->get_components() ) ) {
				p_components.append( pComponent->get_name() );
			}
			m_kit_components[ sDrumkitName ] = p_components;
		}
	}

	drumkitList->insertSeparator( drumkitList->count() );

	QStringList userDrumkits = Filesystem::usr_drumkit_list();
	for (int i = 0; i < userDrumkits.size(); ++i) {
		QString absPath = Filesystem::usr_drumkits_dir() + userDrumkits.at(i);
		Drumkit *info = Drumkit::load( absPath, false );
		if (info) {
			m_pDrumkitInfoList.push_back( info );
			drumkitList->addItem( info->get_name() );
			QStringList p_components;
			for ( auto pComponent : *(info->get_components() ) ) {
				p_components.append(pComponent->get_name());
			}
			m_kit_components[info->get_name()] = p_components;
		}
	}

	/*
	 * If the export dialog was called from the soundlibrary panel via right click on
	 * a soundlibrary, the variable preselectedKit holds the name of the selected drumkit
	 */
	if ( m_preselectedKitLookup == Filesystem::Lookup::system ) {
		m_sPreselectedKit.append( m_sSysDrumkitSuffix );
	}

	int index = drumkitList->findText( m_sPreselectedKit );
	if ( index >= 0) {
		drumkitList->setCurrentIndex( index );
	}
	else {
		drumkitList->setCurrentIndex( 0 );
	}

	on_drumkitList_currentIndexChanged( drumkitList->currentText() );
	on_versionList_currentIndexChanged( 0 );
}
