// $Id$
/** \file btree_mainpage.h
 * Contains the doxygen mainpage comment
 */

/*
 * STX B+ Tree Implementation
 * Copyright (C) 2007 Timo Bingmann
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/** \mainpage STX B+ Tree README

\author Timo Bingmann (Mail: tb a-with-circle idlebox dot net)
\date 2007-04-26

\section sec1 Summary

The STX B+ Tree package is a set of C++ template classes implementing a
B<sup>+</sup> tree key/data container in main memory. The classes are designed
as drop-in replacements of the STL containers set, map, multiset and multimap
and follow their interfaces very closely. By packing multiple value pairs into
each node of the tree, the B<sup>+</sup> tree requires fewer heap allocations
and utilizes cache-line effects better than the standard red-black binary
tree. The tree algorithms are based on the implementation in Cormen, Leiserson
and Rivest's Introduction into Algorithms, Jan Jannink's paper and other
algorithm resources. The classes contain extensive assertion and verification
mechanisms to ensure the implementation's correctness by testing the tree
invariants.

\section sec11 License / Website / Bugs

The current source package can be downloaded from
http://idlebox.net/2007/stx-btree/

The complete source code is released under the GNU Lesser General Public
License v2.1 (LGPL) which can be found in COPYING.

If bugs should become known they will be posted on the above web page together
with patches or corrected versions.

\section sec2 Original Application

The idea originally arose while coding a read-only database which used a huge
map of millions of non-sequential integer keys to 8-byte file offsets. When
using the standard STL red-black tree implementation this would yield millions
of 20-byte heap allocations and very slow search times due to the tree's
height. So the original intension was to reduce memory fragmentation and
improve search times. The B<sup>+</sup> tree solves this by packing multiple
data pairs into one node with a large number of descendant nodes.

Furthermore in computer science lectures it is often stated that using
consecutive bytes in memory would be more cache-efficient, because the CPU's
cache levels always fetch larger blocks from main memory. So it would be best
to store the keys of a node in one continuous array. This way the inner
scanning loop would be accelerated by benefiting from cache effects and
pipelining speed-ups. Thus the cost of scanning for a matching key would be
lower than in a red-black tree, even though the number of key comparisons are
theoretically larger. This second aspect aroused my academic interest and
resulted in the speed test experiments.

A third inspiration was that no working C++ template implementation of a B+
tree could be found on the Internet.

\section sec3 Implementation Overview

This implementation contains five main classes within the \ref stx namespace
(blandly named Some Template eXtensions). The base class \ref stx::btree
"btree" implements the B<sup>+</sup> tree algorithms using inner and leaf nodes
in main memory. Almost all STL-required function calls are implemented (see
below for the exceptions). The asymptotic time requirements of the STL standard
are theoretically not always fulfilled. However in practice this B<sup>+</sup>
tree performs better than the STL's red-black tree at the cost of using more
memory. See the speed test results below for details.

The base class is then specialized \ref stx::btree_set "btree_set", \ref
stx::btree_multiset "btree_multiset", \ref stx::btree_map "btree_map" and \ref
stx::btree_multimap "btree_multimap" using default template parameters and
facade-functions. These classes are designed to be drop-in replacements for the
corresponding STL containers.

The insertion function splits the nodes on recursion unroll. Erase is largely
based on Jannink's ideas. See http://dbpubs.stanford.edu:8090/pub/1995-19
for his paper on "Implementing Deletion in B<sup>+</sup>-trees".

The two set classes are derived from the base implementation class btree by
specifying an empty struct as data_type. All function are adapted to provide
the inner class with placeholder objects. Note that it is somewhat inefficient
to implement a set or multiset using a B<sup>+</sup> tree: a plain B tree would hold no
extra copies of the keys.

\section sec4 Problem with Separated Key/Data Arrays

The most noteworthy difference to the default red-black tree implementation of
std::map is that the B<sup>+</sup> tree does not hold key and data pair together in
memory. Instead each B<sup>+</sup> tree node has two separate arrays of keys and data
values. This design was chosen to utilize cache-line effects while scanning the
key array.

However it also directly generates many problems in implementing the iterators'
operators, which return references or pointers to value_type composition
pairs. These data/key pairs however are not stored together and thus a
temporary copy must be constructed. This copy should not be written to, because
it is not stored back into the B<sup>+</sup> tree. This effectively prohibits use of many
STL algorithms writing to the B<sup>+</sup> tree's iterators.

\section sec5 Test Suite

The B<sup>+</sup> tree distribution contains an extensive testsuite using
cppunit. According to gcov 89.23% of the btree.h implementation is covered.

\section sec6 STL Incompatibilities

The tree algorithms currently do not use copy-construction. All key/data items
are allocated using the default-constructor and are subsequently only assigned
new data.

Most important are the non-writable operator* and operator-> of the
\ref stx::btree::iterator "iterator". See above for a discussion of the problem on separated key/data
arrays.
Instead of *iter and iter-> use the new functions iter.key() and iter.data()
which return writable references to the key and data values.

The B<sup>+</sup> tree supports only two erase functions:

\code
size_type erase(const key_type &key); // erase all data pairs matching key
bool erase_one(const key_type &key);  // erase one data pair matching key
\endcode

The following STL-required functions are not supported:

\code
void erase(iterator iter);
void erase(iterator first, iterator last);
\endcode

\section sec7 Extensions

Beyond the usual STL interface the B<sup>+</sup> tree classes support some extra goodies.

\code
// Output the tree in a pseudo-hierarchical text dump to std::cout. This
// function requires that BTREE_DEBUG is defined prior to including the btree
// headers. Furthermore the key and data types must be std::ostream printable.
void print() const;

// Run extensive checks of the tree invariants. If a corruption in found the
// program will abort via assert(). See below on enabling auto-verification.
void verify() const;

// Serialize and restore the B<sup>+</sup> tree nodes and data into/from a binary image
// outputted to the ostream. This requires that the key and data types are
// integral and contain no outside pointers or references.
void dump(std::ostream &os) const;
bool restore(std::istream &is);
\endcode

\section sec8 B+ Tree Traits

All tree template classes take a template parameter structure which holds
important options of the implementation. The following structure shows which
static variables specify the options and the corresponding defaults:

\code
struct btree_default_map_traits
{
    /// If true, the tree will self verify it's invariants after each insert()
    /// or erase(). The header must have been compiled with BTREE_DEBUG
    /// defined.
    static const bool	selfverify = false;

    /// If true, the tree will print out debug information and a tree dump
    /// during insert() or erase() operation. The header must have been
    /// compiled with BTREE_DEBUG defined and key_type must be std::ostream
    /// printable.
    static const bool	debug = false;

    /// Number of slots in each leaf of the tree. Estimated so that each node
    /// has a size of about 128 bytes.
    static const int 	leafslots =
                             MAX( 8, 128 / (sizeof(_Key) + sizeof(_Data)) );

    /// Number of slots in each inner node of the tree. Estimated so that each
    /// node has a size of about 128 bytes.
    static const int	innerslots =
                             MAX( 8, 128 / (sizeof(_Key) + sizeof(void*)) );
};
\endcode

*/
