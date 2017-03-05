
//  (C) Copyright Edward Diener 2011-2015
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#include <boost/vmd/elem.hpp>
#include <boost/vmd/is_empty.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/preprocessor/list/at.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

int main()
  {
  
#if BOOST_PP_VARIADICS

  #define BOOST_VMD_REGISTER_ggh (ggh)
  
  #define ANIDENTIFIER ggh
  #define ANUMBER 249
  #define ASEQ (25)(26)(27)
  #define ATUPLE (0,1,2,3,((a,b))((c,d))((e))((f,g,h)))
  #define ATUPLE2 (10,11,12,13)
  #define ATUPLE3 (100,101,102,103)
  #define ATUPLE5 (200,201,202,203)
  #define ALIST (0,(1,(2,(3,BOOST_PP_NIL))))
  #define ANARRAY (3,(a,b,38))
  #define ANARRAY2 (5,(c,d,133,22,15))
  #define ASEQUENCE ANUMBER ALIST ATUPLE ANIDENTIFIER ANARRAY ASEQ
  #define ASEQUENCE2 ANIDENTIFIER ANARRAY2 ALIST ASEQ ATUPLE2
  #define ASEQUENCE3 ASEQ ANIDENTIFIER ATUPLE3 ALIST
  #define ASEQUENCE4
  #define ASEQUENCE5 ALIST ASEQ ATUPLE5 ANIDENTIFIER

  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(0,ASEQUENCE,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(0,ASEQUENCE2,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(0,ASEQUENCE3,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  
  BOOST_TEST(!BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(2,ASEQUENCE,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(2,BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(2,ASEQUENCE,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))),2);
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(3,BOOST_VMD_ELEM(2,ASEQUENCE,BOOST_VMD_TYPE_TUPLE)),3);
  
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(4,ASEQUENCE2,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(4,ASEQUENCE2,BOOST_VMD_TYPE_TUPLE)),11);
  
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(2,BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(2,ASEQUENCE3,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))),102);
  BOOST_TEST(!BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(2,ASEQUENCE3,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(2,ASEQUENCE3,BOOST_VMD_TYPE_TUPLE)),100);
 
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(0,BOOST_VMD_ELEM(0,ASEQUENCE4,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(0,ASEQUENCE4,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST(BOOST_VMD_IS_EMPTY(BOOST_VMD_ELEM(0,ASEQUENCE4,BOOST_VMD_TYPE_TUPLE)));
  
  BOOST_TEST(!BOOST_VMD_IS_EMPTY(BOOST_PP_TUPLE_ELEM(1,BOOST_VMD_ELEM(2,ASEQUENCE5,BOOST_VMD_RETURN_AFTER,BOOST_VMD_TYPE_TUPLE))));
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(2,BOOST_VMD_ELEM(2,ASEQUENCE5,BOOST_VMD_TYPE_TUPLE)),202);
  
  BOOST_TEST_EQ(BOOST_PP_TUPLE_ELEM(3,BOOST_VMD_ELEM(2,ASEQUENCE,BOOST_VMD_TYPE_TUPLE)),3);
  
#else

BOOST_ERROR("No variadic macro support");
  
#endif

  return boost::report_errors();
  
  }
