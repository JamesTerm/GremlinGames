#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace FrameWork::xml;

// Parse from a string
tree::tree( const char	text[], const int size )
	:	m_p_parent( NULL ),
		m_internal_node( this )
{	// Initialize
	ss_init( m_text );
	ss_init( m_type );

	// Go go go
	load_from_string( text, &m_internal_node, size );
}

tree::tree( const wchar_t text[], const int size )
	:	m_p_parent( NULL ),
		m_internal_node( this )
{	// Initialize
	ss_init( m_text );
	ss_init( m_type );

	// Go go go
	load_from_string( text, &m_internal_node, size );
}

// Constructor
tree::tree( void )
	:	m_p_parent( NULL ),
		m_internal_node( this )
{	// Initialize
	ss_init( m_text );
	ss_init( m_type );
}

// Destructor
tree::~tree( void )
{	// Free children
	clear();
}

// Load from a file
bool tree::read_from_file( const wchar_t filename[] )
{
	// Go go go
	clear();
	return load_from_file( filename, &m_internal_node );
}

// Load from a string
bool tree::read_from_string( const char	 text[], const int size )
{
	// Go go go
	clear();
	return load_from_string( text, &m_internal_node, size );
}

bool tree::read_from_string( const wchar_t text[], const int size )
{
	// Go go go
	clear();
	return load_from_string( text, &m_internal_node, size );
}

// Load from a resource
bool tree::read_from_resource( HANDLE hModule, const DWORD resource_ID, const wchar_t* resource_type )
{	// Not yet set
	const char *p_data = NULL;
	size_t size = 0;
	
	// Look for the resource
	HRSRC rsrc = ::FindResourceW( (HMODULE)hModule, MAKEINTRESOURCE( resource_ID ), resource_type );
    if ( !rsrc ) return false;

	// Load in the resource
	HGLOBAL glbl_rsrc = ::LoadResource( (HMODULE)hModule, rsrc );
	if ( !glbl_rsrc ) return false;

	// Get the resource pointer
	p_data = (char*)::LockResource( glbl_rsrc );
	if ( !p_data ) return false;

	size = ::SizeofResource( (HMODULE)hModule, rsrc );
	if ( !size ) return false;

	// Try to read the mesh file.
	read_from_string( p_data, (int)size );

	// Success
	return true;
}

// Free all memory on this node (and all children)
void tree::clear( void )
{	// Remove everything
	remove_all_parameters();
	remove_all_children();
	ss_free( m_text );
	ss_free( m_type );
}

// Get access to the node type 
// e.g.		<DDR> </DDR>
// The node type is DDR
const wchar_t *tree::type( void ) const
{
	return m_type.second;
}

const wchar_t *tree::text( void ) const
{
	return m_text.second;
}

const int tree::no_parameters( void ) const
{
	return (int)m_parameters.size();
}

void tree::set_type( const wchar_t name[] )
{
	ss_set( m_type, name );
}

void tree::set_text( const wchar_t text[] )
{
	ss_set( m_text, text );
}

tree* tree::parent( void )
{	
	return m_p_parent;
}

const tree* tree::parent( void ) const
{
	return m_p_parent;
}

// Get parameter by name, NULL if not found.
const wchar_t* tree::parameter( const wchar_t name[], const wchar_t *p_default ) const
{
	// Look for the parameter
	for( int i=0; i<(int)m_parameters.size(); i++ )
	if ( ( m_parameters[ i ].first.second ) && 
		 ( !::wcscmp( name, m_parameters[ i ].first.second ) ) )
			return m_parameters[ i ].second.second;

	// Return the parameter
	return p_default;
}

const std::pair< const wchar_t*, const wchar_t* > tree::parameter( const int idx ) const
{
	assert( idx < (int)m_parameters.size() );
	std::pair< const wchar_t*, const wchar_t* >	ret;
	ret.first  = m_parameters[ idx ].first .second;
	ret.second = m_parameters[ idx ].second.second;
	return ret;
}

// Get the number of children
const int tree::no_children( void ) const
{	// Get the number of children
	return (int)m_children.size();
}

// Get the child
tree &tree::child( const int idx )
{	assert( idx < (int)m_children.size() );
	return *m_children[ idx ];
}

const tree &tree::child( const int idx ) const
{	assert( idx < (int)m_children.size() );
	return *m_children[ idx ];
}

void tree::ss_init( simple_string& str )
{	
	str.first = 0;
	str.second = NULL;
}

void tree::ss_free( simple_string& str )
{	
	if ( str.second ) 
		::free( str.second );
	str.first = 0;
	str.second = NULL;
}

void tree::ss_set( simple_string& str, const wchar_t text[] )
{	
	// Set it
	ss_set( str, text, (int)::wcslen( text ) );
}

void tree::ss_set( simple_string& str, const wchar_t text[], const int no_chars )
{
	// Free any previous version
	ss_free( str );

	// Now set it
	str.first = no_chars;
	str.second = (wchar_t*)::malloc( sizeof(wchar_t)*( no_chars + 1 ) );
	::memcpy( str.second, text, sizeof(wchar_t)*( no_chars + 1 ) );
}

void tree::ss_add( simple_string& str, const wchar_t text[] )
{	
	// Add it
	ss_add( str, text, (int)::wcslen( text ) );
}

void tree::ss_add( simple_string& str, const wchar_t text[], const int no_chars, const bool terminate )
{
	// Allocate a temporary new string
	str.second = (wchar_t*)::realloc( str.second, sizeof(wchar_t)*( no_chars + str.first + 1 ) );
	::memcpy( str.second + str.first, text, sizeof(wchar_t)*( no_chars + 1 ) );
	if ( terminate ) str.second[ str.first + no_chars ] = 0;
	str.first += no_chars;
}

// Constructor
tree::internal_node::internal_node( tree* p_parent )
	: m_p_parent( p_parent )
{
}
				
// Create a child.
node *tree::internal_node::p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{	// No return value yet
	node *p_ret = NULL;

	// If this is the top of the tree, then we do not process any further.
	if ( ( !m_p_parent->m_p_parent ) && ( !m_p_parent->type() ) )
	{	// Use this node as the root.
		p_ret = this;
	}
	else
	{	// Create a new child parent node
		tree* p_tree = new tree;
		p_tree->m_p_parent = m_p_parent;

		// This is a child
		m_p_parent->m_children.push_back( p_tree );

		// The return value
		p_ret = &p_tree->m_internal_node;
	}

	// Return the result
	return p_ret;
}

// This item is starting to be created
void tree::internal_node::start( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{	// Set the type
	ss_set( m_p_parent->m_type, type );

	// Add the parameters
	assert( !m_p_parent->m_parameters.size() );
	m_p_parent->m_parameters.resize( no_parameters );
	for( int i=0; i<no_parameters; i++ )
	{	// Set the settings	
		ss_set( m_p_parent->m_parameters[ i ].first , p_parameters[ i ].m_p_name );
		ss_set( m_p_parent->m_parameters[ i ].second, p_parameters[ i ].m_p_value );
	}
}

// The current text
void tree::internal_node::add_text( const wchar_t text[], const int no_chars )
{
	// Add in the text
	ss_add( m_p_parent->m_text, text, no_chars, true );
}

// Remove all parameters
void tree::remove_all_parameters( void )
{	for( size_t i=0; i<m_parameters.size(); i++ )
	{	// Free the string memory
		ss_free( m_parameters[ i ].first );
		ss_free( m_parameters[ i ].second );
	}

	// Clear the list
	m_parameters.clear();
}

// Add a parameter
void tree::add_parameter( const wchar_t name[], const wchar_t value[] )
{
	// Add an element
	m_parameters.resize( m_parameters.size()+1 );

	// Set it
	ss_set( m_parameters.back().first, name );
	ss_set( m_parameters.back().second, value );
}

tree* tree::get_child( const wchar_t name[] )
{
	for( int i=0; i<(int)m_children.size(); i++ )
	if ( ( m_children[i] ) && ( !::wcscmp( name, m_children[i]->type() ) ) ) return m_children[i];
	return NULL;
}

const tree* tree::get_child( const wchar_t name[] ) const
{
	for( int i=0; i<(int)m_children.size(); i++ )
	if ( ( m_children[i] ) && ( !::wcscmp( name, m_children[i]->type() ) ) ) return m_children[i];
	return NULL;
}

// Set a parameter
void tree::set_parameter( const int idx, const wchar_t name[], const wchar_t value[] )
{
	// Add blank items
	if ( idx >= (int)m_parameters.size() ) 
	{	// A string to expand with
		std::pair< simple_string, simple_string > null_string;
		ss_init( null_string.first );
		ss_init( null_string.second );
		m_parameters.resize( idx+1, null_string );
	}
	
	// Set the entry
	ss_set( m_parameters[idx].first,  name  );
	ss_set( m_parameters[idx].second, value );
}

void tree::set_parameter( const wchar_t name[], const wchar_t value[] )
{
	// Cycle over all parameters
	for( int i=0; i<(int)m_parameters.size(); i++ ) 
	if ( ( m_parameters[ i ].first.second ) && ( !::wcscmp( m_parameters[ i ].first.second, name ) ) )
	{	// Change the value
		ss_set( m_parameters[ i ].second, value );

		// Success
		return;
	}

	// Not found
	add_parameter( name, value );
}

void tree::safe_set_parameter( const wchar_t name[], const wchar_t value[] )
{
	//check if the string has any esc chars, if so esc_value will be the new string
	//if not it will be null
	const wchar_t *p_esc_value = escape_text_for_XML( value );

	//set the parameter normally with the escape string if one was generated or the
	//original string if no escpae seq was needed
	set_parameter( name, p_esc_value ? p_esc_value : value );

	//delete the new string if it was allocated
	escape_text_for_XML_free( p_esc_value );
}

// Remove all children
void tree::remove_all_children( void )
{
	// Delete all children
	for( size_t i=0; i<m_children.size(); i++ ) delete m_children[ i ];
	m_children.clear();
}

// Add a child
void tree::add_child( tree* p_new_child )
{
	// Add a new child
	m_children.push_back( p_new_child );
}

// Set a child
void tree::set_child( const int idx, tree* p_new_child )
{
	// Add blank items
	if ( idx >= (int)m_children.size() ) 
	{	// Add it in
		m_children.resize( idx+1, NULL );
	}

	// Kill any child that I have
	if ( m_children[ idx ] ) delete m_children[ idx ];
	m_children[ idx ] = p_new_child;
}

bool tree::execute( node *p_node ) const
{	// Allocate enough space for the parameters
	node::parameter *p_parameters = (node::parameter*)_alloca( m_parameters.size()*sizeof(node::parameter) );
	for( int i=0; i<(int)m_parameters.size(); i++ )
	{	p_parameters[i].m_p_name  = m_parameters[i].first.second;
		p_parameters[i].m_p_value = m_parameters[i].second.second;
	}

	// Call the node
	node *p_child = p_node->p_create_child( m_type.second, (int)m_parameters.size(), p_parameters );

	// Start the data
	if ( p_child )
	{	// This item is starting to be created
		p_child->start( m_type.second, (int)m_parameters.size(), p_parameters );

		// Add the text
		p_child->add_text( m_text.second, m_text.first );

		// Recursion
		for( int i=0; i<(int)m_children.size(); i++ )
		if ( m_children[ i ] ) m_children[ i ]->execute( p_child );

		// Finish adding the child
		p_child->end( m_type.second );
	}

	// Finish creation
	p_node->create_child_end( m_type.second, p_child );

	// Finished
	return true;
}

int tree::output_length( void ) const
{
	return recurse_output_length( 0 );
}

int tree::recurse_output_length( const int depth ) const
{
	int ret = 0;

	// Just a paramter entry
	if ( ( m_children.empty() ) && ( !m_text.first ) )
	{	// <ddr/>
		ret += 1;				// <
		ret += m_type.first;	// ddr
		
		for( int i=0; i<(int)m_parameters.size(); i++ )
		{	ret += 1;								//  
			ret += m_parameters[i].first.first;		// param
			ret += 1;								// =
			ret += 1;								// "
			ret += m_parameters[i].second.first;	// param
			ret += 1;								// "
		}

		ret += 1;									// /
		ret += 1;									// >
	}
	else
	{	// <ddr param="">text</ddr>
		ret += 1;				// <
		ret += m_type.first;	// ddr
		
		for( int i=0; i<(int)m_parameters.size(); i++ )
		{	ret += 1;								//  
			ret += m_parameters[i].first.first;		// param
			ret += 1;								// =
			ret += 1;								// "
			ret += m_parameters[i].second.first;	// param
			ret += 1;								// "
		}

		ret += 1;				// >
		ret += m_text.first;	// text

		// Recursion
		for( int i=0; i<(int)m_children.size(); i++ )
		if ( m_children[ i ] ) ret += m_children[ i ]->recurse_output_length( depth+1 );

		// Ending
		ret += 1;				// <
		ret += 1;				// /
		ret += m_type.first;	// ddr
		ret += 1;				// >
	}

	// Finished
	return ret;
}

// Write to output
void tree::output( wchar_t destination[] ) const
{
	recurse_output( destination, 0 );
}

int tree::recurse_output( wchar_t destination[], const int depth ) const
{
#define _add_str_( a )	{ assert( a.second ); if ( a.second ) { ::memcpy( p_destination, a.second, a.first*sizeof(wchar_t) ); p_destination += a.first; } }
#define _add_chr_( a )	{ p_destination[0] = (a); p_destination++; }

	wchar_t *p_destination = destination;

	// Just a paramter entry
	if ( ( m_children.empty() ) && ( !m_text.first ) )
	{	// <ddr/>
		_add_chr_( L'<' );							// <
		_add_str_( m_type );						// ddr
		
		for( int i=0; i<(int)m_parameters.size(); i++ )
		{	_add_chr_( L' ' );						//  
			_add_str_( m_parameters[i].first );		// param
			_add_chr_( L'=' );						// =
			_add_chr_( L'"' );						// "
			_add_str_( m_parameters[i].second );	// param
			_add_chr_( L'"' );						// "
		}

		_add_chr_( L'/' );							// /
		_add_chr_( L'>' );							// >
	}
	else
	{	// <ddr param="">text</ddr>
		_add_chr_( L'<' );							// <
		_add_str_( m_type );						// ddr
		
		for( int i=0; i<(int)m_parameters.size(); i++ )
		{	_add_chr_( L' ' );							//  
			_add_str_( m_parameters[i].first );			// param
			_add_chr_( L'=' );							// =
			_add_chr_( L'"' );							// "
			_add_str_( m_parameters[i].second );		// param
			_add_chr_( L'"' );							// "
		}

		_add_chr_( L'>' );								// >
		if ( m_text.first ) { _add_str_( m_text ); }	// text

		// Recursion
		for( int i=0; i<(int)m_children.size(); i++ )
		if ( m_children[ i ] ) 
			p_destination += m_children[ i ]->recurse_output( p_destination, depth+1 );

		// Ending
		_add_chr_( L'<' );							// <
		_add_chr_( L'/' );							// /
		_add_str_( m_type );						// ddr
		_add_chr_( L'>' );							// >
	}

	// EOS
	if ( !depth ) _add_chr_( 0 );					// EOS

#undef _add_str_
#undef _add_chr_	

	// Return the length
	return (int)( p_destination - destination );
}