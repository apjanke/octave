FCN_FILE_DIRS += deprecated

deprecated_FCN_FILES = \
  deprecated/find_dir_in_path.m \
  deprecated/isstr.m

FCN_FILES += $(deprecated_FCN_FILES)

PKG_ADD_FILES += deprecated/PKG_ADD

DIRSTAMP_FILES += deprecated/$(octave_dirstamp)
