#!/bin/sh

g++ 				\
	-std=c++14		\
	-Wall -Werror	\
	main.cc			\
	-lm -lstdc++	\
	-o oppai		\
					\
&& strip -R .comment ./oppai
