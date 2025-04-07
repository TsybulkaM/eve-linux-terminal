set(SRC 
  eveld_main.c
  eveld_fb.c
  eveld_ops.c
  eveld_mem.c
  Ubuntu_16.c
  logo.c
)
add_eve_ececutable(
  NAME eveld
  SRC ${SRC}
)