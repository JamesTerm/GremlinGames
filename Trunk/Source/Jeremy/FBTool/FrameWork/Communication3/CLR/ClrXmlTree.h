#pragma once

public ref class XmlTree
{
public:
	// Constructor
	XmlTree( const char		xml_data[] );
	XmlTree( const wchar_t	xml_data[] );
	XmlTree( System::String ^ xml_data );

	// Constructor with pointer to unmanagedXMLTree
	XmlTree( unmanaged_xml::tree * ut );

	// Destructor
	~XmlTree( void );

	// Finalizer
	!XmlTree( void );

	// Get Unmanaged Pointer
	unmanaged_xml::tree * GetUnmanagedPointer();

			// General management
			// ******************
				// Free all memory on this node (and all children)
				void Clear( void );

				bool ReadFromFile( System::String ^ filename );

				bool ReadFromString( System::String ^ text );

			// Get tree data
			// *************
				// Get access to the node type 
				// e.g.		<DDR> </DDR>
				// The node type is DDR
				System::String ^ Type( void );

				// Get access to the parameters
				// e.g.		<DDR value="1" another_value="2">
				// There are two parameters. "value","1"   "another_value","2"
				int NoParameters( void );

				// If you access a parameter past the end, NULL is returned.
				System::String ^ ParameterKey( const int idx );
				System::String ^ ParameterValue( const int idx );

				// Get parameter by name, NULL if not found.
				System::String ^ ParameterValue( System::String ^ key );

				// Get the text on this node
				// e.g.		<DDR>hello world</DDR>
				// The text is "hello world"
				System::String ^ Text( void );

				int NoChildren( void );

				XmlTree ^ GetChild( const int idx );

				// Get the first child of a given name.
				// NULL if not found
				XmlTree ^ GetChild( System::String ^ name );

				// Get the parent (might be NULL if this is the root)
				XmlTree ^ GetParent( void );


			// Set tree data
			// *************
				// Set the data type
				void SetType( System::String ^ name );

				// Set the text
				void SetText( System::String ^ text );
				
				// Remove all parameters
				void RemoveAllParameters( void );

				// Add a parameter
				void AddParameter(  System::String ^ name, System::String ^ value );

				// Set a parameter				
				void SetParameter( int idx, System::String ^ name, System::String ^ value );

				// Set a parameter if it exists, *** case sensitive ***
				// Adds it if it does not exist
				void SetParameter( System::String ^ name, System::String ^ value );

				// Remove all children
				void RemoveAllChildren( void );

				// Add a child
				void AddChild( XmlTree ^ newChild );

				// Set a child
				void SetChild( int idx, XmlTree ^ newChild );

			// Generate output
			// ***************
				// Get the output of the xml tree as a String

				System::String ^ GetOutput(void);


private:
	unmanaged_xml::tree * m_pUnmanagedXmlTree;
	bool m_ownsUnmanagedXmlTree;
};