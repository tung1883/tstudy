#include <limits.h>

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums2Size < nums1Size) {
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }

    int left = 0;
    int right = nums1Size;

    while (left <= right) {
        int i = (left + right) / 2;
        int j = (nums1Size + nums2Size + 1) / 2 - i;
        int Aleft = (i == 0) ? INT_MIN : nums1[i - 1];
        int Aright = (i == nums1Size) ? INT_MAX : nums1[i];
        int Bleft = (j == 0) ? INT_MIN : nums2[j - 1];
        int Bright = (j == nums2Size) ? INT_MAX : nums2[j];
        
        if (Aleft <= Bright && Bleft <= Aright) {
            // correct partition found
            if ((nums1Size + nums2Size) % 2 == 0) {
                int leftMax = (Aleft > Bleft) ? Aleft : Bleft;
                int rightMin = (Aright < Bright) ? Aright : Bright;
                return (leftMax + rightMin) / 2.0;
            }

            return  (Aleft > Bleft) ? Aleft : Bleft;
        }

        if (Aleft > Bright) right = i - 1;
        else left = i + 1;
    }

    return 0.0;
}

/* Key notes
- Use the trick return function() to force some desired condition (here is making the first array always the smaller one)
- Think of i as boundary, not as an element in the array, so with length-n array, there will be n+1 boundaries. This handles edge cases better
- Generalize the binary search: we use it when (1) have a sorted array; (2) some condition that can be reached by partioning the array
*/