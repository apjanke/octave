FCN_FILE_DIRS += set

set_FCN_FILES = \
  set/complement.m \
  set/intersect.m \
  set/ismember.m \
  set/setdiff.m \
  set/setxor.m \
  set/union.m \
  set/unique.m

FCN_FILES += $(set_FCN_FILES)

PKG_ADD_FILES += set/PKG_ADD
