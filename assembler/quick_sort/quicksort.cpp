#include <cstdio>
#include <iostream>
using namespace std;
void xchg(long& a, long& b) {
  if (a==b) return;
  a = a ^ b;
  b = a ^ b;
  a = a ^ b;
}



void sort(long * data, unsigned long count) {
  if (count <= 1) return;
  for(int j=0;j<count;j++) cout << data[j] << ' ';
  cout << endl;
  unsigned long lt = 0;
  unsigned long i = 1;
  unsigned long gt = count-1;
  cout << lt << ' ' << i << ' ' << (gt>>1) << endl;
  long v = data[gt>>1];
  xchg(data[0],data[gt>>1]);
  while (i <= gt) {
	int cmp = data[i] - v;
	if (cmp < 0) xchg(data[lt++], data[i++]);
	else if (cmp > 0) xchg(data[i], data[gt--]);
 	else i++;
  } // Now a[lo..lt-1] < v = a[lt..gt] < a[gt+1..hi].
  
  sort(data, lt);
  sort(data + gt + 1, count-gt-1);
} 



long data[5000000];
int main() {
    unsigned long count;
    scanf("%lu", &count);
    for (unsigned long i = 0; i < count; ++i)
        scanf("%ld", &data[i]);
    sort(data, count);
    for (unsigned long i = 0; i < count; ++i)
        printf("%ld ", data[i]);
    printf("\n");
    return 0;
}
