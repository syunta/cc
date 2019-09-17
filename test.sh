#!/usr/bin/env bash

try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [[ "$actual" = "$expected" ]]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 ' 12 + 34 - 5 ;'
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 10 '-10+20;'
try 0 '0==1;'
try 1 '42==42;'
try 1 '0!=1;'
try 0 '42!=42;'
try 1 '0<1;'
try 0 '1<1;'
try 0 '2<1;'
try 1 '0<=1;'
try 1 '1<=1;'
try 0 '2<=1;'
try 1 '1>0;'
try 0 '1>1;'
try 0 '1>2;'
try 1 '1>=0;'
try 1 '1>=1;'
try 0 '1>=2;'
try 1 'a=1;'
try 3 'z=3;'
try 5 'a=2;b=3;a+b;'
try 10 'a=12;b=8;c=2;(a+b)/c;'
try 4 'a = 1; b = a + 3; b;'
try 9 'a=b=c=3 ;a + b + c;'
try 7 'foo = 4; bar = 3; foo + bar;'
try 1 'return 1;'
try 3 'return 3; 5 + 9;'
try 10 'if (4 == 5) 3; 10;'
try 5 'a=3; if (1) if (1) a=5; a;'

echo OK