#.rst:
# Findbluetooth
# ---------------
#
# Find the bluetooth library
#
# Targets
# ^^^^^^^
#
# If successful, the following import target is created.
#
# ``bluetooth::bluetooth``
#
# Cache variables
# ^^^^^^^^^^^^^^^
#
# The following cache variable may also be set to assist/control the operation of this module:
#
# ``BLUETOOTH_ROOT_DIR``
#  The root to search for bluetooth

set(bluetooth_ROOT_DIR
        "${bluetooth_ROOT_DIR}"
        CACHE
        PATH
        "Directory to search for the BlueZ bluetooth library")

find_package(PkgConfig)
pkg_check_modules(PC_BLUETOOTH QUIET bluetooth bluez)

find_path(bluetooth_INCLUDE_DIR
    NAMES bluetooth/bluetooth.h
    PATHS ${bluetooth_ROOT_DIR}
    HINTS ${PC_BLUETOOTH_INCLUDE_DIRS})
find_library(
        bluetooth_LIBRARY
        NAMES bluetooth libbluetooth
        PATHS ${bluetooth_ROOT_DIR}
        HINTS ${PC_BLUETOOTH_LIBDIR})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(bluetooth
        REQUIRED_VARS bluetooth_LIBRARY bluetooth_INCLUDE_DIR)

if (bluetooth_FOUND)
    set(bluetooth_INCLUDE_DIRS ${bluetooth_INCLUDE_DIR})
    set(bluetooth_LIBRARIES ${bluetooth_LIBRARY})

    if (NOT TARGET bluetooth::bluetooth)
        add_library(bluetooth::bluetooth UNKNOWN IMPORTED)
        set_target_properties(bluetooth::bluetooth PROPERTIES
                IMPORTED_LOCATION "${bluetooth_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${bluetooth_INCLUDE_DIR}")
    endif ()
endif()
mark_as_advanced(bluetooth_INCLUDE_DIR bluetooth_LIBRARY)
