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


#include <core/Synth/Synth.h>
#include <core/Basics/Note.h>
#include <core/Globals.h>

#include <cassert>
#include <cmath>

namespace H2Core
{

Synth::Synth()
		: Object()
{
	

	m_pOut_L = new float[ MAX_BUFFER_SIZE ];
	m_pOut_R = new float[ MAX_BUFFER_SIZE ];

	m_fTheta = 0.0;

	m_pAudioOutput = nullptr;
}



Synth::~Synth()
{
	INFOLOG( "DESTROY" );
	delete[] m_pOut_L;
	delete[] m_pOut_R;
}




void Synth::noteOn( Note* pNote )
{
	INFOLOG( "NOTE ON" );
	assert( pNote );

	m_playingNotesQueue.push_back( pNote );
}




void Synth::noteOff( Note* pNote )
{
	INFOLOG( "NOTE OFF - not implemented yet" );
	assert( pNote );

	// delete the older note...
	for ( uint i = 0; i < m_playingNotesQueue.size(); ++i ) {
		Note *pPlayingNote = m_playingNotesQueue[ i ];
		if ( pPlayingNote->get_instrument() == pNote->get_instrument() ) {
			m_playingNotesQueue.erase( m_playingNotesQueue.begin() + i );
			delete pPlayingNote;

			delete pNote;
			pNote = nullptr;
			break;
		}
	}

	ERRORLOG( "note not found" );
}



// perche' viene passata anche la canzone? E' davvero necessaria?
void Synth::process( uint32_t nFrames )
{
	//INFOLOG( "process" );

	// cleanup of the output buffers
	memset( m_pOut_L, 0, nFrames * sizeof( float ) );
	memset( m_pOut_R, 0, nFrames * sizeof( float ) );


	for ( const auto& pPlayingNote :  m_playingNotesQueue ) {
		//pPlayingNote->dumpInfo();

		float fAmplitude = pPlayingNote->get_velocity();
		float fFrequency = TWOPI * 220.0 / 44100.0;

		for ( uint i = 0; i < nFrames; ++i ) {
			float fVal = sin( m_fTheta ) * fAmplitude;
			m_pOut_L[ i ] += fVal;
			m_pOut_R[ i ] += fVal;

			m_fTheta += fFrequency;
		}
	}
}


void Synth::setAudioOutput( AudioOutput* pAudioOutput )
{
	m_pAudioOutput = pAudioOutput;
}



} // namespace H2Core
