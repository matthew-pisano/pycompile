set(CPACK_PACKAGE_NAME "pycompile")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "RPM")

set(CPACK_RPM_PACKAGE_VENDOR "Matthew Pisano")

set(CPACK_RPM_PACKAGE_LICENSE "BSD-3-Clause")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Python compiler")
set(CPACK_RPM_PACKAGE_DESCRIPTION "The pycompile Python compiler and standard library")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _buildhost anonymized")
set(CPACK_RPM_PACKAGE_URL "https://github.com/matthew-pisano/pycompile")

set(CPACK_RPM_PACKAGE_REQUIRES
        "llvm-devel >= 21, python3-devel >= 3.14"
)

include(CPack)
