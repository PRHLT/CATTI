/* Jorge Civera <jorcisai@iti.upv.es>, Aug 2007 */
/** \file 
\brief \d Library implementing a hash data structure and its operations */
#ifndef HASH_INCLUDED
#define HASH_INCLUDED

/******************************************************************************/
/**************************CONSTANTS*******************************************/
/******************************************************************************/

#define HASH_KEY_NOT_FOUND -1

/*****************************************************************************/
/*************************TYPE DEFINITION*************************************/
/*****************************************************************************/
      
typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

/** \d Element in hash list */
typedef struct hlist{
  void *key;          /**< \d Generic pointer to key of this hash element */
  int  key_length;    /**< \d Length of key */
  void *data;         /**< \d Generic pointer to data */ 
  struct hlist *next; /**< \d Pointer to next element in hash list */
} hlist_t;

/** \d Data structure to store a hash table */
typedef struct hash{
  hlist_t **hash_table; /**< \d Array of hash lists */
  int useful_bits;      /**< \d Just a mask to shorten hash key to hash table */
  int table_size;       /**< \d Size of hash table */
} hash_t;

/*****************************************************************************/
/*******************************FUNCTIONS*************************************/
/*****************************************************************************/

/** \d mix -- mix 3 32-bit values reversibly.

\verbatim 
--------------------------------------------------------------------
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a
  structure that could supported 2x parallelism, like so:
      a -= b;
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
\endverbatim
*/
#define mix(a,b,c){ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}
      
/** \d Hash function parameters: key, length of the key, the previous hash or an arbitrary value

\verbatim
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.  
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have HASHSIZE(10) elements.
  
If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);
   
By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.
      
See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
\endverbatim
*/  
ub4 hash_function(register ub1 *k, register ub4 length, register ub4 initval);

/** \d Create hash table of size table_size */
hash_t *create_hash(int table_size);

/** \d Return the pointer to the data with given key */
void *get_hash(void *key, int key_length, hash_t *hs);

/** \d Insert data and return the pointer to it */
void *set_hash(void *key, int key_length, hash_t *hs, void *data);

/** \d Remove element from hash table */
void *del_hash(hash_t *hs, char *key, int key_length);

/** \d Free hash table and elements that are pointers */
void destroy_hash(hash_t *hs, int is_pointer);

#endif

