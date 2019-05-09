# Copyright (c) 2019-present, Tencent, Inc.
# All rights reserved.
#
# Author: Haojie Pan
# Email: jasonhjpan@tencent.com
#

CXX = c++
CXXFLAGS = -std=c++11 -Iinclude
OBJS = myutils.o fastnewwords.o main_fast.o

fastnewwords: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o fastnewwords; rm *.o

main_fast.o: src/main_fast.cc include/fastnewwords.h
	$(CXX) $(CXXFLAGS) -c src/main_fast.cc

fastnewwords.o: src/fastnewwords.cc include/fastnewwords.h include/myutils.h  \
				include/tsl/array-hash/array_growth_policy.h include/tsl/array-hash/array_hash.h \
				include/tsl/array-hash/array_map.h include/tsl/array-hash/array_map.h \
				include/tsl/htrie_set.h include/tsl/htrie_hash.h include/tsl/htrie_map.h
	$(CXX) $(CXXFLAGS) -c src/fastnewwords.cc

myutils.o: src/myutils.cc include/myutils.h
	$(CXX) $(CXXFLAGS) -c src/myutils.cc

clean:
	rm -rf *.o *.gcno *.gcda fastnewwords