Purpose:

Library FastFish designed for organize full-text search in the application. Current 
API presented for the C++ language, in the future planned to add wrappers for use 
with other languages. 

Key Features:

Search queries use Standard Boolean model. This provides a high-speed query execution. 
Example: for simple conjunctive queries and the size of the index order of a few 
gigabytes, you can do tens of thousands of queries/second on a each processors core. 

Supported a static ranking for documents, i.e. rank of the document specified in 
the time of indexing. 

High-speed indexing - a few tens megabytes/second per processors core. 

Search is possible with index, which splitted on several segments. With a large 
number of segments, query performance decreases slightly. For example, when the 
index size of about gigabyte, splitting it to 50 segments slows down the speed 
of simple conjunctive queries about 2 times. 

Each segment of the index all-sufficient, possible any combination of segments, 
if they have the same type. 

Loading segment is a lightweight operation, reading the entire file is not required. 
The data of segment used in searches through memory mapping. This restrict a total 
size of the index when search performed on 32-bit platform (depending from the OS, 
usually 2GB). 

FastFish is oriented on tasks, in which the index is fit entirely in RAM or slightly 
exceeds its capacity, no more than a few times. When searching on an index that 
do not fit into memory and stored on the HDD, the speed drops substantially. 
When used the SSD, speed decrease is minor.

Request consists of elementary operations that can be combined with logical operators 
and grouped with any degree of nesting. 

FastFish working with text in UTF-8 format. Internal data is stored directly in UTF-8. 
Supported the complete set of Unicode characters, i.e. including the characters 
above U+FFFF. Supported case-insensitive search. 

System requirements:
	Hardware platform: 32 or 64-bit machines, both little-endian and big-endian. 
		Possible to use the platform with strict requirements for data alignment. 		
	OS: POSIX-compatible environment or Win32/64.					  	
	Compiler: currently, building and operability has been tested on GCC 4.3.4 
		and 4.5.0, MSVC 2005, ICC 11.1/Windows.
	
	The format of the index depends only from the endianness of platform. 
	I.e. index builded on the little-endian platform, will be inapplicable for 
	searching on big-endian platform and vice versa. All other platform characteristics 
	(32/64 bit, OS, compiler) does not affect the binary format of the index. 

Limitations:
	* 2 147 483 647 documents per one segment 
	* 65 536 segments
	* 128 fields in the index 
	* 255 elementary operations in the query 
	* 32 words in each elementary operation 
	* 65 535 ranks 
	* 65 536 - length of the value in data field 
	* 65 536 - length of the word in the texts for the search fields 
	Length of the text in any search field of a document is not restricted. 

//////////////////////////////////////////////////////////////////////////////	
Short description
//////////////////////////////////////////////////////////////////////////////	

Structure of index 
	
The index consists of a set of fields and a set of documents. The type of index 
is determined by field set. Index files of the same type can be considered as 
segments of a single index. The order of the segments defined by load order of 
files in object Searcher. 

The fields are classified into two categories - search fields and data fields. 
Only search field may be used to search for documents. Data, assigned to the search 
field in the indexing time, in its original form are not stored. To receive the data, 
related to the document, use the data field. 

Search Field:

Currently, the search field has only one possible type - the text. Text field 
consists of a set of words. Splitting of the text into words is made by library 
on the basis of the list of possible delimiters. The list of delimeters is one of 
the attribute of the type of field. 

A text field can be of three main subtypes (enum TextIndexType): 
	* Exact - searching for a word that exactly equal to the argument. For example, 
		if the document contains the word "abracadabra", it will be found 
		when you specify in query  "abracadabra" entirely. Index with this 
		field type will take up less total space and quick to build. 
	* Prefix - prefix search on the words. For example, a document with "abracadabra" 
		can be found not only in with entire word, but also with "abracadab" 
		or "abraca". Constructing prefix index takes a little longer time 
		(usually 10-20%), but the index size increases significantly. 
	* Infix - search for any substring of the word. For example, "abracadabra" 
		to be found with the "brac", "cada", "cadabra", etc. Infix index 
		construction time is increased significantly (usually 2-3 times) 
		compared with Prefix, and index size too. 

Data Field:

The current version supports 2 types of data fields - with a fixed data length 
(template TplFieldDataFix) and variable data length (template TplFieldDataVar). 
FieldDataFix can be used, for example, to store the number, associated with the document. 
FieldDataVar suitable for storing any data of irregular length, for example, 
the title of the document. 

Searching

In the FastFish queries, data separated from the text of the query and passed through 
arguments to the object Query. Query constructed from a text expression, operating 
with the indices of the fields and the indices of the arguments. 

When searching in the text field performed, argument is a string with one or more 
words. As words delimiters treated the same characters, as when the index build 
(for the corresponding field). 

For the text field there are two types of elementary operations in the queries: 
	* Documents, which includes all the words from the argument 
		- is denoted by the letter a (from "all") 
	* Documents, which includes some words from the argument 
		- is denoted by the letter s (from "some") 
		
The format of the elementary operations - "%OP%%FIELD%.%ARG%", where: 
	* %OP% - the operation code, "a" or "s" 
	* %FIELD% - the index of the field 
	* %ARG% - the index of the argument 

Query example: "a3,0" - find the documents, which in a field #3 includes all of 
the words from the argument #0. 

Elementary operations can be combined using "&" (conjunction, a binary operator), 
"|" (disjunction, a binary operator), "!" (negation, unary operator), and grouped 
with parentheses. In the absence of parentheses, operators have priority 
(in descending order): ! & |. 

Query example: "!(a3,0 | s2,1)" - all documents does not belong to the following set: 
documents that contain all the words from the argument #0 in field #3, 
or containing some word from the argument #1 in field #2. 

Number of nested levels of parentheses is not limited. Only the number of elementary 
operations is limited by 255. 

The search is executed by passing arguments to object Query. Result is the sequence 
of documents. For position in sequence the rank has the greatest priority, then 
a segment, then document number in a segment. The segment and number are considered 
upside-down, i.e. the last added document of the last segment is in sequence the 
first among documents of the same rank.

Computation of the next document in sequence happens only when Sequence::Next() 
called. Exact length of sequence can be found only by sampling all documents. 
Approximate length can be obtained from attribute Sequence::ApproximateLength. 
If the search includes large set of words, ApproximateLength can differ significantly 
from exact length. In special cases when whole query is reduced to search by one word, 
ApproximateLength returns exact length of  sequence.

Error handling 

Errors can be classified into 2 categories: runtime and development time. 
Runtime errors include input-output errors, incorrect input data or lack of resources 
such as memory. Development time errors include logical errors in the code of the library 
or incorrect usage of library.

Runtime errors:

In case of runtime error FastFish throws C++ exceptions. All methods and functions 
has the specification of thrown exceptions of one of two types:
	* throw() - the method doesn't throw exceptions
	* ffThrowAll - the method throw any exceptions inherited from std::runtime_error 
		(classes Except* from Ñommon/Types.h)

The special case is handling the out of memory. The library mainly ignores uprising 
bad_alloc - methods in which there can be throws this exception, have the specification 
throw(). Weak guarantee on handling bad_alloc are supported only, i.e. the resources 
occupied with FastFish objects, will be released at removal of objects. But after 
throw bad_alloc, the object can remain in an invalid state.

This approach is used to achieve high performance, because maintaining the correct 
state of the objects after bad_alloc would lead to overheads, including at normal 
execution.

If is necessary to split the index into segments when insufficient memory occured, 
it is possible to implement with using std::set_new_handler and installing a 
out of memory handler. Before the creation of an index it is necessary to reserve 
a "emergency" memory block, sufficient for the finalize segment. Then out of memory 
handler called, block released, a current data is stored as segment, the "emergency" 
block allocated again and the process of indexing going further.

An example of such processing of insufficient memory can be seen in the example 
applications GeoImport and WikiImport. If specified the "-sm" command line argument, 
index is splitted into segments automatically, when out of memory happens.

Passing to FastFish incorrect UTF-8 sequences is not considered as a critical error. 
The library always validate UTF-8 after splitting text into words (both at index 
creation time, and at search time). If there incorrect sequences occured - the word 
is restricted to the last valid character, an exception is not generated. In this 
case FastFish write (optional) warnings into log.

Development-time errors (AKA bugs):

The library can be built in two possible configurations, release or debug. 
FastFish behavior in these configurations are different.

In debug configuration, in run-time checked the possibility of logical errors through 
assertions. If any assertion is fail, the program is halted and called the debugger. 

For errors which are a consequence of incorrect usage FastFish, into the log wrote 
record of lsBugUser type, suggests that the caller does wrong. Check of assertions 
is one of the reasons that the debug version runs much slower than the release version.

In the release configuration assertions is not checked, it is assumed that all of them 
are true. In case of violation of any assertion, behavior of the FastFish undefined.

It should be noted that, when searching, as bugs classified also incorrect data 
in the index. I.e. it is assumed that the reason of incorrect data is a logical 
error in index builder code. Situation where the index file after the construction 
has been changed due to external reasons not considered, because complete validation 
of the index is complex task. In the release version, the library checks in runtime 
only the signature of file.

Logging:

Information about errors, warnings, and some additional data can be write into 
the log in text format. By default, the log is stderr, but for any type of log 
entries it can be replaced by any other object of type FILE*. Entries in the log 
is classified into several types, all listed in enum LogStream (Common/Log.h).

Logging records of any type can be turned off, as at the compilation time (by removal 
the type from constant LogStream::lsAll), and at runtime (by specifying (FILE*)0 
in SetLog call). For records of type LogStream::lsBug, release/debug configuration 
has a higher priority than log managing, i.e., in the release config this type of 
records does not appear in any case.

//////////////////////////////////////////////////////////////////////////////

License 

FastFish licensed under the General Public License version 2. The text of the 
license can be found in the file license.txt, distributed with the library, 
or online at www.gnu.org .

Development priorities: 
	0. The bugs. Those functions that are implemented, should work properly. 
	1. Query performance. 
	2. Speed of indexing. 
	3. Minimizing the size of the index.
	Paragraphs 1 and 2 are generally not in conflict.
	
The policy of version increment: 

The version number looks like major.minor.revision. 
	* Increment revision does not affect the index format and API. The new 
		revision could mean better performance or bug fixes. 
	* Increment minor version may affect the format of the index, but the old 
		index files must be compatible with code compiled with the new version 
		of FastFish. New index files may be incompatible with code compiled 
		with an older version FastFish. Possible additions to the API with 
		preserve backward compatibility, i.e., to recompile with the new 
		version, caller code no need to change. 
	* Increment major version means a complete loss of compatibility with the 
		old index file and/or significant changes in the API, i.e., to build 
		with a new version, the calling code may require changes. 

Version of this document - 0.0.0 

Version History:
	0.0.0 - initial release
