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

#include "utility.h"

#include "settings.h"
#include "context.hpp"

#include <QEventLoop>

QStringList utility::split( const QString& e,char token,bool skipEmptyParts )
{
	if( skipEmptyParts ){
		#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
			return e.split( token,QString::SkipEmptyParts ) ;
		#else
			return e.split( token,Qt::SkipEmptyParts ) ;
		#endif
	}else{
		return e.split( token ) ;
	}
}

QList< QByteArray > utility::split( const QByteArray& e,char token )
{
	return e.split( token ) ;
}

#ifdef Q_OS_LINUX

bool utility::platformIsLinux()
{
	return true ;
}

bool utility::platformIsOSX()
{
	return false ;
}

bool utility::platformIsWindows()
{
	return false ;
}

int utility::terminateProcess( unsigned long )
{
	return 0 ;
}

QString utility::python3Path()
{
	return QStandardPaths::findExecutable( "python3" ) ;
}

#endif

#ifdef Q_OS_MACOS

QString utility::python3Path()
{
	return QStandardPaths::findExecutable( "python3" ) ;
}

bool utility::platformIsOSX()
{
	return true ;
}

bool utility::platformIsLinux()
{
	return false ;
}

bool utility::platformIsWindows()
{
	return false ;
}

int utility::terminateProcess( unsigned long )
{
	return 0 ;
}
#endif

#ifdef Q_OS_WIN

#include <windows.h>

template< typename Function,typename Deleter,typename ... Arguments >
auto unique_rsc( Function&& function,Deleter&& deleter,Arguments&& ... args )
{
	using A = std::remove_pointer_t< std::result_of_t< Function( Arguments&& ... ) > > ;
	using B = std::decay_t< Deleter > ;

	return std::unique_ptr< A,B >( function( std::forward< Arguments >( args ) ... ),
				       std::forward< Deleter >( deleter ) ) ;
}

template< typename Type,typename Deleter >
auto unique_ptr( Type type,Deleter&& deleter )
{
	return unique_rsc( []( auto arg ){ return arg ; },
			   std::forward< Deleter >( deleter ),type ) ;
}

int utility::terminateProcess( unsigned long pid )
{
	FreeConsole() ;

	if( AttachConsole( pid ) == TRUE ) {

		/*
		 * Add a fake Ctrl-C handler for avoid instant kill in this console
		 * WARNING: do not revert it or current program will also killed
		 */

		SetConsoleCtrlHandler( nullptr,true ) ;

		if( GenerateConsoleCtrlEvent( CTRL_C_EVENT,0 ) == TRUE ){

			return 0 ;
		}
	}

	return 1 ;
}

static HKEY _reg_open_key( const char * subKey,HKEY hkey )
{
	HKEY m ;
	REGSAM wow64 = KEY_QUERY_VALUE | KEY_WOW64_64KEY ;
	REGSAM wow32 = KEY_QUERY_VALUE | KEY_WOW64_32KEY ;
	unsigned long x = 0 ;

	if( RegOpenKeyExA( hkey,subKey,x,wow64,&m ) == ERROR_SUCCESS ){

		return m ;

	}else if( RegOpenKeyExA( hkey,subKey,x,wow32,&m ) == ERROR_SUCCESS ){

		return m ;
	}else{
		return nullptr ;
	}
}

static void _reg_close_key( HKEY hkey )
{
	if( hkey != nullptr ){

		RegCloseKey( hkey ) ;
	}
}

static QByteArray _reg_get_value( HKEY hkey,const char * key )
{
	if( hkey != nullptr ){

		DWORD dwType = REG_SZ ;

		std::array< char,4096 > buffer ;

		std::fill( buffer.begin(),buffer.end(),'\0' ) ;

		auto e = reinterpret_cast< BYTE * >( buffer.data() ) ;
		auto m = static_cast< DWORD >( buffer.size() ) ;

		if( RegQueryValueEx( hkey,key,nullptr,&dwType,e,&m ) == ERROR_SUCCESS ){

			return { buffer.data(),static_cast< int >( m ) } ;
		}
	}

	return {} ;
}

static QString _readRegistry( const QString& subKey,const char * key,HKEY hkey )
{
	auto s = unique_rsc( _reg_open_key,_reg_close_key,subKey.toUtf8().constData(),hkey ) ;

	return _reg_get_value( s.get(),key ) ;
}

QString utility::python3Path()
{
	QString a = "Software\\Python\\PythonCore\\3.%1\\InstallPath" ;

	for( int s = 9 ; s >= 0 ; s-- ){

		auto c = _readRegistry( a.arg( QString::number( s ) ),"ExecutablePath",HKEY_CURRENT_USER ) ;

		if( !c.isEmpty() ){

			return c ;
		}
	}

	for( int s = 9 ; s >= 0 ; s-- ){

		auto c = _readRegistry( a.arg( QString::number( s ) ),"ExecutablePath",HKEY_LOCAL_MACHINE ) ;

		if( !c.isEmpty() ){

			return c ;
		}
	}

	return {} ;
}

bool utility::platformIsWindows()
{
	return true ;
}

bool utility::platformIsLinux()
{
	return false ;
}

bool utility::platformIsOSX()
{
	return false ;
}

#endif

bool utility::platformIsNOTWindows()
{
	return !utility::platformIsWindows() ;
}

static void _add( QMenu * menu,const QStringList& args )
{
	for( const auto& it : args ){

		auto a = it ;

		a.replace( "Best-audiovideo(",QObject::tr( "Best-audiovideo" ) + "(" ) ;
		a.replace( "Best-audio(",QObject::tr( "Best-audio" ) + "(" ) ;

		auto b = a.lastIndexOf( '(' ) ;

		if( b != -1 ){

			auto m = a.mid( 0,b ) ;
			auto mm = a.mid( b + 1 ) ;
			mm.truncate( mm.size() - 1 ) ;
			menu->addAction( m )->setObjectName( mm ) ;
		}else{
			menu->addAction( it )->setObjectName( it ) ;
		}
	}
}

QMenu * utility::details::sMo( const Context& ctx,
			       const QStringList& opts,
			       bool addClear,
			       QPushButton * w )
{
	auto m = w->menu() ;

	if( m ){

		m->deleteLater() ;
	}

	auto menu = new QMenu( w ) ;

	auto& translator = ctx.Translator() ;
	auto& settings = ctx.Settings() ;

	translator::entry ss( QObject::tr( "Preset Options" ),"Preset Options","Preset Options" ) ;
	auto ac = translator.addAction( menu,std::move( ss ) ) ;

	ac->setEnabled( false ) ;

	menu->addSeparator() ;

	_add( menu,settings.presetOptionsList() ) ;

	if( !opts.empty() ){

		menu->addSeparator() ;

		translator::entry ss( QObject::tr( "Found Options" ),"Found Options","Found Options" ) ;

		_add( translator.addMenu( menu,std::move( ss ) ),opts ) ;
	}

	if( addClear ){

		menu->addSeparator() ;

		translator::entry ss( QObject::tr( "Clear Options" ),
						   translator::CLEAROPTIONS,
						   translator::CLEAROPTIONS ) ;

		translator.addAction( menu,std::move( ss ) ) ;

		translator::entry sx( QObject::tr( "Clear Screen" ),
						   translator::CLEARSCREEN,
						   translator::CLEARSCREEN ) ;

		translator.addAction( menu,std::move( sx ) ) ;
	}

	w->setMenu( menu ) ;

	return menu ;
}

bool utility::hasDigitsOnly( const QString& e )
{
	for( const auto& it : e ){

		if( !( it >= '0' && it <= '9'  ) ){

			return false ;
		}
	}

	return true ;
}

QString utility::homePath()
{
	if( utility::platformIsWindows() ){

		return QDir::homePath() + "/Desktop" ;
	}else{
		return QDir::homePath() ;
	}
}

void utility::waitForOneSecond()
{
	utility::wait( 1 ) ;
}

void utility::wait( int time )
{
	QEventLoop e ;

	utility::Timer( 1,[ & ]( int counter ){

		if( counter == time ){

			e.exit() ;
			return true ;
		}else{
			return false ;
		}
	} ) ;

	e.exec() ;
}
