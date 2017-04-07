#!/bin/sh

${CXX} 				\
	-std=c++0x		\
	-Wall -Werror	\
	main.cc			\
	-lm -lstdc++	\
	-o oppai		\
					\
&& strip -R .comment ./oppai
