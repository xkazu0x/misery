////////////////////////////////
// NOTE: Handle Type Functions

internal FP_Handle
fp_handle_zero(void) {
  FP_Handle result = {0};
  return(result);
}

internal b32
fp_handle_match(FP_Handle a, FP_Handle b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}
