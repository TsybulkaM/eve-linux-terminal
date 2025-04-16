set(SRC 
  eveld_main.c
  eveld_fb.c
  eveld_ops.c
  IBM_plex.c
  logo.c
)
add_eve_ececutable(
  NAME eveld
  SRC ${SRC}
)