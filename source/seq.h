/* seq.h
 *
 * A structure to hold a sequence of elements.
 *
 *   PUBLIC VARIABLES
 *
 *     none!
 *
 *   CONSTRUCTORS
 *
 *     seq()               Create an empty sequence
 *
 *   PUBLIC FUNCTIONS
 *
 *     add( x )            Add x to the end of the sequence
 *     remove()            Remove the last element of the sequence
 *     remove( i )         Remove the i^{th} element of the sequence (expensive)
 *     shift( i )          Shift right everything starting at position i
 *     operator [i]        Returns the i^{th} element (starting from 0)
 *     exists( x )         Return true if x exists in sequence, false otherwise
 *     clear()             Deletes the whole sequence
 *     findIndex( x )      Find the index of element x, or -1 if it doesn't exist
 */


#ifndef SEQ_H
#define SEQ_H

#include "headers.h"

#include <iostream>
#include <stdlib.h>

using namespace std;


template<class T> class seq {

  int storageSize;
  int numElements;
  T  *data;

public:

  seq() {			// constructor
    storageSize = 2;
    numElements = 0;
    data = new T[ storageSize ];
  }

  seq( int n ) {		// constructor
    storageSize = n;
    numElements = 0;
    data = new T[ storageSize ];
  }

  ~seq() {			// destructor
    delete [] data;
  }

  seq( const seq<T> & source ) { // copy constructor

    storageSize = source.storageSize;
    numElements = source.numElements;
    data = new T[ storageSize ];
    for (int i=0; i<numElements; i++)
      data[i] = source.data[i];
  }

  void remove() {
    if (numElements == 0) {
      cerr << "remove: Tried to remove element from empty sequence\n";
      exit(-1);
    }

    numElements = numElements - 1;
  }

  void remove( int i );
  void shift( int i );
  void compress();

  int size() const {
    return numElements;
  }

  T & operator [] ( int i ) const {
    if (i >= numElements || i < 0) {
      cerr << "element: Tried to access an element beyond the range of the sequence: "
	   << i << "(numElements = " << numElements << ")\n";
      { int i = 0;
      int j = 1 / i;
      j = j;
      }
      exit(-1);
    }
    return data[ i ];
  }

  void clear() {
    delete [] data;
    storageSize = 1;
    numElements = 0;
    data = new T[ storageSize ];
  }

  seq<T> & operator = (const seq<T> &source) { // assignment operator
    storageSize = source.storageSize;
    numElements = source.numElements;
    delete [] data;
    data = new T[ storageSize ];
    for (int i=0; i<numElements; i++)
      data[i] = source.data[i];
    return *this;
  }

  void add( const T &x );
  int findIndex( const T &x );
  bool exists( const T &x );
};


// Add an element to the end of the sequence

template<class T>
void 
seq<T>::add( const T &x )

{
  // No storage left?  If so, double the storage

  if (numElements == storageSize) {
    T *newData;

    newData = new T[ storageSize * 2 ];
    for (int i=0; i<storageSize; i++)
      newData[i] = data[i];
    storageSize = storageSize * 2;
    delete [] data;
    data = newData;
  }

  // Store the element in the next available position

  data[ numElements ] = x;
  numElements++;
}


// Compress the array

template<class T>
void 
seq<T>::compress()

{
  T *newData;

  if (numElements == storageSize)
    return;

  newData = new T[ numElements ];
  for (int i=0; i<numElements; i++)
    newData[i] = data[i];
  storageSize = numElements;
  delete [] data;
  data = newData;
}


// Find and return an element

template<class T>
bool 
seq<T>::exists( const T &x )

{
  for (int i=0; i<numElements; i++)
    if (data[i] == x)
      return true;

  return false;
}


// Find and return the *index* of an element

template<class T>
int 
seq<T>::findIndex( const T &x )

{
  for (int i=0; i<numElements; i++)
    if (data[i] == x)
      return i;

  return -1;
}


// Shift a suffix of the sequence to the right by one

template<class T>
void 
seq<T>::shift( int i )

{
  if (i < 0 || i >= numElements) {
    cerr << "remove: Tried to shift element " << i
	 << " from a sequence of " << numElements << " elements \n";
    exit(-1);
  }

  if (numElements == storageSize) {
    T *newData;

    newData = new T[ storageSize * 2 ];
    for (int i=0; i<storageSize; i++)
      newData[i] = data[i];
    storageSize = storageSize * 2;
    delete [] data;
    data = newData;
  }

  for (int j=numElements; j>i; j--)
    data[j] = data[j-1];

  numElements++;
}


// Shift a suffix of the sequence to the left by one

template<class T>
void 
seq<T>::remove( int i )

{
  if (i < 0 || i >= numElements) {
    cerr << "remove: Tried to remove element " << i
	 << " from a sequence of " << numElements << " elements \n";
    { int i = 0;
    int j = 1 / i;
    }
    exit(-1);
  }

  for (int j=i; j<numElements-1; j++)
    data[j] = data[j+1];

  numElements--;
}



#endif
