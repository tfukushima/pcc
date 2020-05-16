#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./pcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert_funcall() {
  expected="$1"
  input="$2"

  cc -c test.c
  ./pcc "$input" > tmp.s
  cc -o tmp tmp.s test.o
  ./tmp
  actual="$?"

  if [[ "$actual" = "$expected" ]]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0  "main() { return 0; }"
assert 42 "main() { return 42; }"
assert 21 "main() { return 5+20-4; }"
assert 41 "main() { return  12 + 34 - 5 ; }"
assert 47 'main() { return 5+6*7; }'
assert 15 'main() { return 5*(9-6); }'
assert 4  'main() { return (3+5)/2; }'
assert 42 "main() { return +42; }"
assert 42 "main() { return +2*+3*+7; }"
assert 42 "main() { return -(-42); }"
assert 42 "main() { return -(-(-(-42)); }"
assert 42 "main() { return -6*-7; }"
assert 42 "main() { return -2 * +3 * -7; }"
assert 1  "main() { return 42==42; }"
assert 0  "main() { return 42!=42; }"
assert 0  "main() { return 42<42; }"
assert 1  "main() { return 42<=42; }"
assert 0  "main() { return 42>42; }"
assert 1  "main() { return 42<=42; }"
assert 1  "main() { return 1 * 2 * 3 * 7 == 42; }"
assert 1  "main() { return 6 * 7 == -7 * (-6); }"
assert 1  "main() { return 1 * 2 * 3 * 7 == 42; }"
assert 1  "main() { return (44 - 20 * 3 / 2) * 3 == 42; }"
assert 1  "main() { return (42!=42)==0; }"
assert 1  "main() { return 42!=42==0; }"
assert 42 "main() { a=42; return a; }"
assert 42 "main() { a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=w=x=y=z=42; return z; }"
assert 1  "main() { a=0; b=1; return b; }"
assert 25 "main() { a=0;b=1;c=2;d=3;e=4;f=5;g=6;h=7;i=8;j=9;k=10;l=11;m=12;n=13;o=14;p=15;q=16;r=17;s=18;t=19;u=20;v=21;w=22;x=23;y=24;z=25; return z; }"
assert 1  "main() { a=0; return a + 1;}"
assert 25 "main() { a=0;b=a+1;c=b+1;d=c+1;e=d+1;f=e+1;g=f+1;h=g+1;i=h+1;j=i+1;k=j+1;l=k+1;m=l+1;n=m+1;o=n+1;p=o+1;q=p+1;r=q+1;s=r+1;t=s+1;u=t+1;v=u+1;w=v+1;x=w+1;y=x+1;z=y+1; return z; }"
assert 3  "main() { a = 1; b = 2; c = a + b; return c; }"
assert 42 "main() { a=1; b=2; c=3; d=7; return a*b*c*d; }"
assert 6  "main() { foo = 1; bar = 2 + 3; return foo + bar; }"
assert 0  "main() { variablewithlongname = 1; anothervariablewithyetlongname = -1; return variablewithlongname + anothervariablewithyetlongname; }"
assert 42 "main() { a1ph4numname = 42; return a1ph4numname; }"
assert 42 "main() { foo_bar = 21; baz_ = 2; quxx = foo_bar * baz_; return quxx; }"
assert 3  "main() { a = 1; b = 2; return a+b; }"
assert 42 "main() { if (1) return 42; }"
assert 42 "main() { if (0) return 1; else return 42; }"
assert 42 "main() { a = 0; b = 0; if (a == b) return 42; else return 1; }"
assert 42 "main() { a = 0; b = 0; if (a < b) return 1; else return 42; }"
assert 10 "main() { a = 0; while (a < 10) a = a + 1;  return a; }"
assert 0  "main() { a = 10; while (a > 0) a = a - 1; return a; }"
assert 20 "main() { a = 0; for (i = 0; i < 10; i = i + 1) a = a + 2; return a; }"
assert 0  "main() { a = 10; for (i = 0; i < 10; i = i + 1) a = a - 1; return a; }"
assert 0  "main() { a = 0; for (i = 0; i < 0;)  a = a + 1; return a; }"
assert 10 "main() { i = 0; for (;i < 10;) i = i + 1; return i; }"
assert 1  "main() { { a = 0; b = 1; return (a + b); } }"
assert 42 "main() { if (0 < 1) { a = 42; return a; } else { return 1; } }"
assert 89 "main() { a = 0; b = 1; for (i = 0; i < 10; i = i + 1) { tmp = b; b = a + b; a = tmp; } return b; }"
assert 42 "main() { {{{{{ return 42; }}}}} }"

assert_funcall 42 "main() { return foo(); }"
assert_funcall 1  "main() { return bar(0, 1); }"
assert_funcall 14 "main() { return bar(1*2, 3*4); }"
assert_funcall 42 "main() { a = 3*7; b = -3*(-7); return bar(a, b); }"

assert 42 "h() { return 21; } main() { n = 2; return n * h(); }"
assert 42 "inc(n) { if (n == 42) return n; return inc(n + 1); } main() { return inc(0); }"
assert 42 "z() { a = 42; return a; } main() { return z(); }"
assert 4  "square(x) { return x * x; } main() { return square(2); }"
assert 4  "square(x) { return x * x; } main() { n = 2; return square(n); }"
assert 10 "add4(a, b, c, d) { return a + b + c + d; } main() { return add4(1,2,3,4); }"
assert 0 "zero(n) { if (n == 0) { return 0; } else { return zero(n-1); } } main() { return zero(10); }"
assert 10 "add3(a, b, c) { d = 4; return a+b+c+d; } main() { return add3(1, 2, 3); }"
assert 42 "ans(a, b) { a = 21; b = 21; return a+b; } main() { return ans(1, 2); }"
assert 42 "ans(a, b) { if (a + b >= 42) { return a+b; } else { return ans(a+1, b+1); } } main() { return ans(1, 1); }"
assert 120 "factorial(n) { if (n == 1) return 1; return n * factorial(n-1); } main() { return factorial(5); }"
assert 55 "fib(n) { if (n == 0) return 0; if (n == 1) return 1; return fib(n-1) + fib(n-2); } main () { return fib(10); }"

assert 3 "main() { x = 3; y = &x; return *y; }"
assert 3 "main() { x = 3; y = 5; z = &y + 8; return *z; }"

echo OK
