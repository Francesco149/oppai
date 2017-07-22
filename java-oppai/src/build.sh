cxxflags="$CXXFLAGS"

if [ $(uname) = "Darwin" ]; then
	brew_prefix=$(brew --prefix)
	cxxflags="$cxxflags -I/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Headers"
	cxxflags="$cxxflags -I/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers"
	${CXX:-clang++}							\
		-I$brew_prefix/opt/openssl/include	\
		-L$brew_prefix/opt/openssl/lib		\
		$cxxflags							\ 
        -std=c++98                         	\
        -pedantic                          	\
        -O2                                	\
        $@                                 	\
        -Wno-variadic-macros               	\
        -Wall -Werror                      	\
        Build.cpp                          	\
        -lm -lstdc++                       	\
        -lcrypto                           	\
        -o liboppai.jnilib
else
	cxxflags="$cxxflags -I/usr/lib/jvm/java-8-openjdk-amd64/include"
	cxxflags="$cxxflags -I/usr/lib/jvm/java-8-openjdk-amd64/include/linux"
	${CXX:-g++}					\
		-std=c++98				\
		-fPIC					\
		-shared					\
		$cxxflags				\
		-pedantic            	\
		-O2                  	\
        $@                   	\
        -Wno-variadic-macros 	\
        -Wall -Werror        	\
        Build.cpp            	\
        -lm -lstdc++         	\
        -lcrypto             	\
        -o liboppai.so
    [ -f ./oppai ] && strip -R .comment ./oppai
fi