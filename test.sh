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

try 0 'int main () { 0; }'
try 42 'int main () { 42; }'
try 21 'int main () { 5+20-4; }'
try 41 'int main () {  12 + 34 - 5 ; }'
try 47 'int main () { 5+6*7; }'
try 15 'int main () { 5*(9-6); }'
try 4 'int main () { (3+5)/2; }'
try 10 'int main () { -10+20; }'
try 0 'int main () { 0==1; }'
try 1 'int main () { 42==42; }'
try 1 'int main () { 0!=1; }'
try 0 'int main () { 42!=42; }'
try 1 'int main () { 0<1; }'
try 0 'int main () { 1<1; }'
try 0 'int main () { 2<1; }'
try 1 'int main () { 0<=1; }'
try 1 'int main () { 1<=1; }'
try 0 'int main () { 2<=1; }'
try 1 'int main () { 1>0; }'
try 0 'int main () { 1>1; }'
try 0 'int main () { 1>2; }'
try 1 'int main () { 1>=0; }'
try 1 'int main () { 1>=1; }'
try 0 'int main () { 1>=2; }'
try 1 'int main () { int a; a=1; return a; }'
try 3 'int main () { int z; z=3; return z; }'
try 5 'int main () { int a; int b; a=2;b=3;a+b; }'
try 5 "int main () { int a; int b; int c; int d; int e; a=1; b=1; c=1; d=1; e=1; return a+b+c+d+e; }"
try 10 'int main () { int a; int b;int c; a=12;b=8;c=2;(a+b)/c; }'
try 4 'int main () { int a; int b; a = 1; b = a + 3; b; }'
try 9 'int main () { int a; int b; int c; a=b=c=3 ;a + b + c; }'
try 7 'int main () { int foo; int bar; foo = 4; bar = 3; foo + bar; }'
try 1 'int main () { return 1; }'
try 3 'int main () { return 3; 5 + 9; }'
try 10 'int main () { if (4 == 5) 3; 10; }'
try 5 'int main () { int a; a=3; if (1) if (1) a=5; a; }'
try 1 'int main () { int a; a=0; if (1) a=a+1; else a=a+2; a; }'
try 2 'int main () { int a; a=0; if (0) a=a+1; else a=a+2; a; }'
try 10 'int main () { int a; a=0; while (a < 10) a=a+1; a; }'
try 20 'int main () { int a; int i; a=0; for (i=0; i<10; i=i+1) a=a+2; a; }'
try 3 'int main () { int a; a=0; for (;a < 1;) a=a+3; a; }'
try 2 "int main () { { 1; 2; } }"
try 15 "int main () { int a; int i; a=0; for(i=0; i<5; i=i+1) {a=a+1; a=a+1; a=a+1;} a; }"
try 10 "int add () { 10; } int main () { add(); }"
try 6 "int sub (int x, int y) { x - y; } int main () { sub(7, 1); }"
try 0 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(0);}"
try 1 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(1);}"
try 1 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(2);}"
try 2 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(3);}"
try 3 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(4);}"
try 5 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(5);}"
try 8 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(6);}"
try 13 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(7);}"
try 21 "int fib(int n){if(n==0){0;}else{if(n==1){1;}else{fib(n-1)+fib(n-2);}}} int main(){fib(8);}"
try 3 "int main () { int x; int *y; x=3; y=&x; *y; }"
try 3 "int main () { int x; int *y; y=&x; *y=3; return x; }"
try 7 "int main () { int x; int *y; int **z; y = &x; z = &y; **z = 7; return x; }"
try 5 'int main() { int x; int y; x=3; y=5; return *(&x+1); }'
try 3 'int main() { int x; int y; x=3; y=5; return *(&y-1); }'
try 3 'int main() { int x; int y; x=3; y=5; return *(&y-(3-2)); }'
try 7 'int main() { int x; int y; x=3; y=5; *(&x+1)=7; return y; }'
try 4 'int main() { return sizeof(100); }'
try 4 'int main() { return sizeof(4+9); }'
try 8 'int main() { int *x; return sizeof(x); }'
try 4 'int main() { return sizeof(sizeof(12)); }'
try 40 'int main() { int a[10]; return sizeof(a); }'
try 8 'int main() { int a[10]; return sizeof(&a); }'
try 1 "int main() { int a[2]; *a = 1; *a; }"
try 3 "int main() { int a[2]; *a = 1; *(a+1)=2; int *p; p=a; return *p + *(p+1); }"

echo OK