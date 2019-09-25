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

try 0 'main () { 0; }'
try 42 'main () { 42; }'
try 21 'main () { 5+20-4; }'
try 41 'main () {  12 + 34 - 5 ; }'
try 47 'main () { 5+6*7; }'
try 15 'main () { 5*(9-6); }'
try 4 'main () { (3+5)/2; }'
try 10 'main () { -10+20; }'
try 0 'main () { 0==1; }'
try 1 'main () { 42==42; }'
try 1 'main () { 0!=1; }'
try 0 'main () { 42!=42; }'
try 1 'main () { 0<1; }'
try 0 'main () { 1<1; }'
try 0 'main () { 2<1; }'
try 1 'main () { 0<=1; }'
try 1 'main () { 1<=1; }'
try 0 'main () { 2<=1; }'
try 1 'main () { 1>0; }'
try 0 'main () { 1>1; }'
try 0 'main () { 1>2; }'
try 1 'main () { 1>=0; }'
try 1 'main () { 1>=1; }'
try 0 'main () { 1>=2; }'
try 1 'main () { int a; a=1; }'
try 3 'main () { int z; z=3; }'
try 5 'main () { int a; int b; a=2;b=3;a+b; }'
try 10 'main () { int a; int b;int c; a=12;b=8;c=2;(a+b)/c; }'
try 4 'main () { int a; int b; a = 1; b = a + 3; b; }'
try 9 'main () { int a; int b; int c; a=b=c=3 ;a + b + c; }'
try 7 'main () { int foo; int bar; foo = 4; bar = 3; foo + bar; }'
try 1 'main () { return 1; }'
try 3 'main () { return 3; 5 + 9; }'
try 10 'main () { if (4 == 5) 3; 10; }'
try 5 'main () { int a; a=3; if (1) if (1) a=5; a; }'
try 1 'main () { int a; a=0; if (1) a=a+1; else a=a+2; a; }'
try 2 'main () { int a; a=0; if (0) a=a+1; else a=a+2; a; }'
try 10 'main () { int a; a=0; while (a < 10) a=a+1; a; }'
try 20 'main () { int a; int i; a=0; for (i=0; i<10; i=i+1) a=a+2; a; }'
try 3 'main () { int a; a=0; for (;a < 1;) a=a+3; a; }'
try 2 "main () { { 1; 2; } }"
try 15 "main () { int a; int i; a=0; for(i=0; i<5; i=i+1) {a=a+1; a=a+1; a=a+1;} a; }"
try 10 "add () { 10; } main () { add(); }"
try 6 "sub (x, y) { x - y; } main () { sub(7, 1); }"
try 0 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(0);}"
try 1 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(1);}"
try 1 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(2);}"
try 2 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(3);}"
try 3 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(4);}"
try 5 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(5);}"
try 8 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(6);}"
try 13 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(7);}"
try 21 "fib(n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} main(){fib(8);}"
try 3 "main () { int x; int y; x=3; y=&x; *y; }"

echo OK