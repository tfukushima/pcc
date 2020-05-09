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

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 42 "+42;"
assert 42 "+2*+3*+7;"
assert 42 "-(-42);"
assert 42 "-(-(-(-42));"
assert 42 "-6*-7;"
assert 42 "-2 * +3 * -7;"
assert 1 "42==42;"
assert 0 "42!=42;"
assert 0 "42<42;"
assert 1 "42<=42;"
assert 0 "42>42;"
assert 1 "42<=42;"
assert 1 "1 * 2 * 3 * 7 == 42;"
assert 1 "6 * 7 == -7 * (-6);"
assert 1 "1 * 2 * 3 * 7 == 42;"
assert 1 "(44 - 20 * 3 / 2) * 3 == 42;"
assert 1 "(42!=42)==0;"
assert 1 "42!=42==0;"
assert 42 "a=42;"
assert 42 "a=b=c=d=e=f=g=h=i=j=k=l=m=n=o=p=q=r=s=t=u=v=w=x=y=z=42;"
assert 1 "a=0; b=1;"
assert 25 "a=0;b=1;c=2;d=3;e=4;f=5;g=6;h=7;i=8;j=9;k=10;l=11;m=12;n=13;o=14;p=15;q=16;r=17;s=18;t=19;u=20;v=21;w=22;x=23;y=24;z=25;"
assert 1 "a=0;a+1;"
assert 25 "a=0;b=a+1;c=b+1;d=c+1;e=d+1;f=e+1;g=f+1;h=g+1;i=h+1;j=i+1;k=j+1;l=k+1;m=l+1;n=m+1;o=n+1;p=o+1;q=p+1;r=q+1;s=r+1;t=s+1;u=t+1;v=u+1;w=v+1;x=w+1;y=x+1;z=y+1;"
assert 3 "a = 1; b = 2; c = a + b;"
assert 42 "a=1; b=2; c=3; d=7; a*b*c*d;"
assert 6 "foo = 1; bar = 2 + 3; foo + bar;"
assert 0 "variablewithlongname = 1; anothervariablewithyetlongname = -1; variablewithlongname + anothervariablewithyetlongname;"
assert 42 "a1ph4numname = 42; a1ph4numname;"
assert 42 "foo_bar = 21; baz_ = 2; quxx = foo_bar * baz_;"
assert 0 "return 0;"
assert 42 "return 42;"
assert 3 "a = 1; b = 2; return a+b;"
assert 42 "if (1) 42;"
assert 42 "if (0) 1; else 42;"
assert 42 "a = 0; b = 0; if (a == b) 42; else 1;"
assert 42 "a = 0; b = 0; if (a < b) 1; else 42;"
assert 10 "a = 0; while (a < 10) a = a + 1; a;"
assert 0 "a = 10; while (a > 0) a = a - 1; a;"

echo OK
