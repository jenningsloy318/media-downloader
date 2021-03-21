/*
 *
 *  Copyright (c) 2021
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generic.h"

generic::generic()
{
}

std::unique_ptr< engines::engine::functions > generic::Functions() const
{
	return std::make_unique< generic::functions >() ;
}

generic::functions::~functions()
{
}

void generic::functions::processData( const engines::engine& engine,
				      QStringList& outPut,
				      QByteArray data )
{
	engines::engine::functions::processData( engine,outPut,std::move( data ) ) ;
}

void generic::functions::updateDownLoadCmdOptions( const engines::engine& engine,
						   const QString& quality,
						   const QStringList& userOptions,
						   QStringList& urls,
						   QStringList& ourOptions )
{
	Q_UNUSED( userOptions )
	Q_UNUSED( urls )

	if( !engine.optionsArgument().isEmpty() ){

		ourOptions.append( engine.optionsArgument() ) ;
	}

	if( !quality.isEmpty() ){

		ourOptions.append( quality ) ;
	}
}