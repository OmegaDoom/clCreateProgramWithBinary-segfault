__kernel
void childKernel(__global int *a, __global int *b , int tid)
{
  b[tid] = a[tid] * 2;
}

__kernel
void krnl(queue_t queue, __global int *a, __global int *b)
{
  uint tid = get_global_id(0);
  kernel_enqueue_flags_t flags = CLK_ENQUEUE_FLAGS_NO_WAIT;
  ndrange_t child_ndrange = ndrange_1D(1);
  void (^child_krnl_block)(void) = ^{ childKernel(a, b, tid); };
  enqueue_kernel(queue, flags, child_ndrange, child_krnl_block);
}
