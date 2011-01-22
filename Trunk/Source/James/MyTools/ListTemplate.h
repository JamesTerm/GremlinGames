#ifndef __LIST_TEMPLATE__
#define __LIST_TEMPLATE__

template<class L>
void Swap(L &a,L &b)
{ L c=a; a=b; b=c; }


/*!	\class		tList
 *	\ingroup	ControlDLL
 *	\brief		A Dynamic list template that manages simple arrayed lists
 *
 *	Warning: Do not instantiate classes that have virtual functions.  It is possible to instantiate structs or classes
 *  which do not contain virtual functions.  Use pointers for classes which contain virtual functions.
 */
//****************************************************************************************************************************************************8
// This list is optimised for contraction and expansion, but at the cost of 4 extra bytes
template<	typename L , 
			bool ExpandOnly = false , 
			size_t ExpandSizeBlockSize = 1 >
class tList
{	public:		// Cas to a pointer ( this allows it to work somewhat like a string ! )
				inline operator L* ( void ) const
					{	return m_Memory;
					}

				inline L* GetPtr( void ) const
					{	return m_Memory;
					}

				inline L* GetPtrEnd( void ) const
					{	return m_Memory+NoItems();
					}

				inline void CheckPtr( const void *Ptr ) const
					{	assert( (L*)Ptr >= m_Memory );
						assert( (L*)Ptr <= (m_Memory+NoItems()-1) );
					}

				// Get the lnegth
				inline size_t NoItems( void ) const 
					{	return m_NoItems; 
					}

				//! Handy
				L* New( const size_t NoItemsToAdd = 1 ) 
					{	const size_t OldListSize = m_NoItems;
						if ( m_NoItems+NoItemsToAdd <= m_NoItems_Allocated ) m_NoItems+=NoItemsToAdd;
						else SetSize( m_NoItems+NoItemsToAdd ); 
						return m_Memory+OldListSize; 
					}

				//! Add an item to the list
				L *Add( const L &Value ) 
					{	const size_t OldSize = m_NoItems;
						if ( m_NoItems+1 <= m_NoItems_Allocated ) m_NoItems++;
						else SetSize( m_NoItems+1 ); 
						memcpy( &m_Memory[OldSize] , &Value , sizeof( L ) );
						return &m_Memory[OldSize];
					}

				L *Add( const size_t NoItemsToAdd , const L *Value ) 
					{	const size_t OldSize=NoItems();
						if ( m_NoItems+NoItemsToAdd <= m_NoItems_Allocated ) m_NoItems+=NoItemsToAdd;
						else SetSize( m_NoItems+NoItemsToAdd ); 
						memcpy( &m_Memory[OldSize] , Value , sizeof( L )*NoItemsToAdd );
						return &m_Memory[OldSize];
					}

					/* ADJC TODO - mwatkins had to take this out to compile under .NET.  Has to do w/ 32 bit aligned list elements
				L *Add( const size_t NoItemsToAdd , const L Value1 , ... ) 
					{	const size_t OldSize=NoItems();
						if ( m_NoItems+NoItemsToAdd <= m_NoItems_Allocated ) m_NoItems+=NoItemsToAdd;
						else SetSize( m_NoItems+NoItemsToAdd ); 
						memcpy( &m_Memory[OldSize] , &Value1 , sizeof(L)*NoItemsToAdd );
						return &m_Memory[OldSize];
					}
					*/

					
				// Insert an item at a given position
					void AddAtPosition( const L &NewItem , const size_t Position=( size_t )-1 , bool InOrder = true )
					{	// Insert the item at the end of the list
						if ( /*( Position==( size_t )-1 )||*/( Position>=NoItems() ) )
								Add( NewItem );
						else
						{	// Increase the list length by one
							SetSize( NoItems()+1 );
							
							// Now memmove
							if (InOrder)
								// Keep the list order constant
								for( size_t i=NoItems()-1 ; i>Position ; i-- )
									m_Memory[i] = m_Memory[i-1];
							else
								// Simple exchange so that it goes into position
								m_Memory[ NoItems()-1 ] = m_Memory[Position];
								
							// Add this item
							m_Memory[Position]=NewItem;
						}
					}

				// Delete a particular entry
				bool DeleteEntry( size_t i ) 
					{	memcpy( &m_Memory[i],&m_Memory[NoItems()-1],sizeof( L ) ); 
						if (!ExpandOnly)
								SetSize( NoItems()-1 ); 
						else	m_NoItems--;

						return true; 
					}

				// Delete an entry on the list
				bool DeleteEntryInOrder( size_t i ) 
					{	if ( !NoItems() ) return false; 
						for( ;i<NoItems()-1;i++ ) 
							memcpy( &m_Memory[i],&m_Memory[i+1],sizeof( L ) ); 
						if (!ExpandOnly)
								SetSize( NoItems()-1 ); 
						else	m_NoItems--;

						return true; 
					}

				// Get a position on the list
				size_t GetPosition( const L &Value ) const 
					{	for( size_t i=0;i<NoItems();i++ ) 
						if ( m_Memory[i]==Value ) 
							return i; 
						return ( size_t )-1; 
					}

				bool Exists( const L &Value ) const
					{	return (GetPosition(Value)!=(size_t)-1);
					}

				// Deleta a value from the list
				bool Delete( const L &Value ) 
					{	size_t Posn=GetPosition( Value ); 
						if ( Posn==( size_t )-1 ) return false; 
						DeleteEntry( Posn ); 
						return true; 
					}

				bool DeleteInOrder( const L &Value ) 
					{	size_t Posn=GetPosition( Value ); 
						if ( Posn==( size_t )-1 ) return false; 
						DeleteEntryInOrder( Posn ); 
						return true; 
					}


				// Copy the list
				void CopyFrom( const tList *From ) 
					{	SetSize( From->NoItems() ); 
						memcpy( ( L* )*this , ( L* )*From ,NoItems()*sizeof( L ) ); 
					}

				// This will swap with a different list. This is pretty quick !
				void SwapFrom( tList *From ) 
					{	Swap( m_NoItems , From->m_NoItems );
						Swap( m_NoItems_Allocated , From->m_NoItems_Allocated );
						Swap( m_Memory , From->m_Memory );
					}

				//! Remove all entries from the list
				void DeleteAll( bool AlwaysFree = !ExpandOnly ) 
					{	SetSize( 0 , AlwaysFree ); 
					}

				//! Set the size of the list
				inline bool SetSize(	size_t NewNoItems , 
										bool ContractList = !ExpandOnly , 
										bool PreserveContents = true ) 
					{	// Store the new size
						m_NoItems = NewNoItems;
					
						// This is the size of the block that we actually want
						if ( !ContractList )					// inlined out
							NewNoItems = max( NewNoItems , m_NoItems_Allocated );
							
						// Apply the size increates
						if ( ExpandSizeBlockSize!=1 )			// inlined out
							// If ExpandSizeBlockSize is a power of two then this is very fast
							NewNoItems = ( ( NewNoItems + ExpandSizeBlockSize - 1 ) / ExpandSizeBlockSize ) * ExpandSizeBlockSize;
						
						// No change in size ?
						if ( NewNoItems == m_NoItems_Allocated ) 
							// Success
							return true;
							
						// ReAllocate the block of memory
						m_NoItems_Allocated = NewNoItems;
						
						// If preserving the contents is not important, use this !
						if ( (!PreserveContents) && (m_Memory) )	// inlined out when PreserveContents==false
						{	NewTek_free(m_Memory);
							m_Memory=NULL;
						}

						// Allocate the memory
						m_Memory = (L*)NewTek_realloc( m_Memory , m_NoItems_Allocated*sizeof(L) ); 						
						
						// Success
						return true;
					}

				//! Build the list of a given size
				tList( size_t Size=0 ) : 
					m_NoItems( 0 ), 
					m_NoItems_Allocated( 0 ),
					m_Memory( NULL )
					{	SetSize( Size );
					}

				//! Destructor
				~tList( void ) 
					{	SetSize( 0 , true );
					}

	protected:	// The number of items currently on the list
				size_t m_NoItems;
				size_t m_NoItems_Allocated;

				// The memory for all the items
				L* m_Memory;
};

#endif __LIST_TEMPLATE__