/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef MIXERLINE_H
#define MIXERLINE_H

#include <QtGui>
#include <QtWidgets>

#include <core/Object.h>
#include <core/Globals.h>
#include <core/Preferences.h>

class Fader;
class MasterFader;
class PanFader;
class Button;
class InstrumentPropertiesDialog;
class InstrumentNameWidget;
class LCDDisplay;
class Rotary;
class WidgetWithInput;
class LED;
class ClickableLabel;

#include "../Widgets/PixmapWidget.h"
#include "../Widgets/WidgetWithScalableFont.h"

class InstrumentNameWidget : public PixmapWidget, protected WidgetWithScalableFont<8, 10, 12>
{
	H2_OBJECT
	Q_OBJECT

public:
	explicit InstrumentNameWidget(QWidget* parent);
	~InstrumentNameWidget();

	void	setText(QString text);
	QString text();

	void	mousePressEvent( QMouseEvent * e ) override;
	void	mouseDoubleClickEvent( QMouseEvent * e ) override;

public slots:
	void onPreferencesChanged( bool bAppearanceOnly );
	
signals:
	void	clicked();
	void	doubleClicked();

protected:
	virtual void paintEvent(QPaintEvent *ev) override;

private:
	int			m_nWidgetWidth;
	int			m_nWidgetHeight;
	QString		m_sInstrName;
	/** Used to detect changed in the font*/
	QString m_sLastUsedFontFamily;
	/** Used to detect changed in the font*/
	H2Core::Preferences::FontSize m_lastUsedFontSize;
};

///
/// A mixer strip
///
class MixerLine: public PixmapWidget
{
	H2_OBJECT
	Q_OBJECT

public:
	MixerLine(QWidget* parent, int nInstr);
	~MixerLine();

	void	updateMixerLine();

	bool	isMuteClicked();
	void	setMuteClicked(bool isClicked);

	bool	isSoloClicked();
	void	setSoloClicked(bool isClicked);

	float	getVolume();
	void	setVolume(float value);

	void	setPeak_L( float peak );
	float	getPeak_L();

	void	setPeak_R( float peak );
	float	getPeak_R();

	void	setName(QString name) {     m_pNameWidget->setText( name );        }
	QString getName() {      return m_pNameWidget->text();        }

	float	getPan();
	void	setPan(float value);

	int		getActivity() {	return m_nActivity;	}
	void	setActivity( uint value ) {	m_nActivity = value;	}

	void	setPlayClicked( bool clicked );

	void	setFXLevel( uint nFX, float fValue );
	float	getFXLevel( uint nFX );

	void	setSelected( bool bIsSelected );

signals:
	void	muteBtnClicked(MixerLine *ref);
	void	soloBtnClicked(MixerLine *ref);
	void	volumeChanged(MixerLine *ref);
	void	instrumentNameClicked(MixerLine *ref);
	void	instrumentNameSelected(MixerLine *ref);
	void	noteOnClicked(MixerLine *ref);
	void	noteOffClicked(MixerLine *ref);
	void	panChanged(MixerLine *ref);
	void	knobChanged(MixerLine *ref, int nKnob);

public slots:
	void	playSampleBtnClicked();
	void	muteBtnClicked();
	void	soloBtnClicked();
	void	faderChanged(WidgetWithInput *ref);
	void	panChanged(WidgetWithInput *ref);
	void	knobChanged(WidgetWithInput *ref);
	void	nameClicked();
	void	nameSelected();

private:
	uint	m_nWidth;
	uint	m_nHeight;
	bool	m_bIsSelected;

	uint	m_nActivity;
	uint	m_nPeakTimer;
	float	m_fMaxPeak;
	float	m_nFalloff;
		
	Fader *					m_pFader;
	Rotary*					m_pPanRotary;
	InstrumentNameWidget *	m_pNameWidget;
	Button *			m_pMuteBtn;
	Button *			m_pSoloBtn;
	Button *				m_pPlaySampleBtn;
	LED*				m_pTriggerSampleLED;
	LED*				m_pSelectionLED;
	Rotary *				m_pFxRotary[MAX_FX];

	LCDDisplay *			m_pPeakLCD;
};

class ComponentMixerLine: public PixmapWidget
{
	H2_OBJECT
	Q_OBJECT

public:
	ComponentMixerLine(QWidget* parent, int CompoID);
	~ComponentMixerLine();

	void	updateMixerLine();

	bool	isMuteClicked();
	void	setMuteClicked(bool isClicked);

	bool	isSoloClicked();
	void	setSoloClicked(bool isClicked);

	float	getVolume();
	void	setVolume(float value);

	void	setPeak_L( float peak );
	float	getPeak_L();

	void	setPeak_R( float peak );
	float	getPeak_R();

	void	setName(QString name) {     m_pNameWidget->setText( name );        }
	QString getName() {      return m_pNameWidget->text();        }

	int		getComponentID(){ return m_nComponentID; }

signals:
	void	muteBtnClicked(ComponentMixerLine *ref);
	void	soloBtnClicked(ComponentMixerLine *ref);
	void	volumeChanged(ComponentMixerLine *ref);

public slots:
	void	soloBtnClicked();
	void	muteBtnClicked();
	
	void	faderChanged(WidgetWithInput *ref);


private:
	int		m_nComponentID;
	uint	m_nWidth;
	uint	m_nHeight;
	bool	m_bIsSelected;

	uint	m_nActivity;
	uint	m_nPeakTimer;
	float	m_fMaxPeak;
	float	m_nFalloff;
		
	Fader *					m_pFader;
	InstrumentNameWidget *	m_pNameWidget;
	Button *			m_pMuteBtn;
	Button *			m_pSoloBtn;
	LCDDisplay *			m_pPeakLCD;
};

class MasterMixerLine: public PixmapWidget
{
	H2_OBJECT
	Q_OBJECT

public:
	explicit MasterMixerLine(QWidget* parent);
	~MasterMixerLine();

	void	updateMixerLine();

	float	getVolume();
	void	setVolume(float value);

	void	setPeak_L(float peak);
	float	getPeak_L();

	void	setPeak_R(float peak);
	float	getPeak_R();


signals:
	void	volumeChanged(MasterMixerLine *ref);


public slots:
	void	faderChanged( WidgetWithInput* pRef);
	void	rotaryChanged( WidgetWithInput *pRef );
	void	muteClicked();

private:
	uint	m_nWidth;
	uint	m_nHeight;

	uint	m_nPeakTimer;
	float	m_fMaxPeak;
	float	m_nFalloff;
		
	Fader*			m_pFader;
	Fader*			m_pMasterFader;

	ClickableLabel* m_pMasterLbl;
	ClickableLabel* m_pHumanizeLbl;
	ClickableLabel* m_pSwingLbl;
	ClickableLabel* m_pTimingLbl;
	ClickableLabel* m_pVelocityLbl;

	LCDDisplay *	m_pPeakLCD;

	Rotary *		m_pSwingRotary;
	Rotary *		m_pHumanizeTimeRotary;
	Rotary *		m_pHumanizeVelocityRotary;

	Button *	m_pMuteBtn;
};

class LadspaFXMixerLine : public PixmapWidget
{
	H2_OBJECT
	Q_OBJECT

public:
	explicit LadspaFXMixerLine(QWidget* parent);
	~LadspaFXMixerLine();

	bool	isFxActive();
	void	setFxActive( bool active );
		
	void	setPeaks( float fPeak_L, float fPeak_R );
	void	getPeaks( float *fPeak_L, float *fPeak_R );
	void	setName( QString name );
		
	float	getVolume();
	void	setVolume( float value );

public slots:
	void activeBtnClicked();
	void editBtnClicked();
	void rotaryChanged( WidgetWithInput* ref);

signals:
	void activeBtnClicked( LadspaFXMixerLine *ref );
	void editBtnClicked( LadspaFXMixerLine *ref );
	void volumeChanged( LadspaFXMixerLine *ref);

private:
	float			m_fMaxPeak;
	Button *	m_pActiveBtn;
	Button *		m_pEditBtn;
	Rotary *		m_pRotary;
	LCDDisplay *	m_pNameLCD;
	ClickableLabel* m_pReturnLbl;
};


#endif
