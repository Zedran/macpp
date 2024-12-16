if (BUNDLE)
	string(REPLACE "_" "." CURL_VERSION_DOTS "${CURL_VERSION}")

	include(FetchContent)
	FetchContent_Declare(
		CURL
		DOWNLOAD_EXTRACT_TIMESTAMP OFF
		URL https://github.com/curl/curl/releases/download/${CURL_VERSION}/${CURL_VERSION_DOTS}.tar.xz
		URL_HASH SHA256=c7ca7db48b0909743eaef34250da02c19bc61d4f1dcedd6603f109409536ab56
	)

	set(BUILD_CURL_EXE              OFF)
	set(BUILD_LIBCURL_DOCS          OFF)
	set(BUILD_SHARED_LIBS           OFF)
	set(BUILD_STATIC_LIBS            ON)
	set(BUILD_TESTING               OFF)
	set(CURL_DISABLE_INSTALL         ON)

	set(CURL_USE_LIBPSL             OFF)
	set(CURL_USE_LIBSSH             OFF)
	set(CURL_USE_LIBSSH2            OFF)
	set(USE_LIBIDN2                 OFF)

	set(CURL_DISABLE_AWS             ON)
	set(CURL_DISABLE_COOKIES         ON)
	set(CURL_DISABLE_KERBEROS_AUTH   ON)
	set(CURL_DISABLE_NETRC           ON)
	set(CURL_DISABLE_NTLM            ON)
	set(CURL_DISABLE_PARSEDATE       ON)
	set(CURL_DISABLE_PROGRESS_METER  ON)
	set(HTTP_ONLY                    ON)

	FetchContent_MakeAvailable(CURL)
else()
	find_package(CURL REQUIRED)
endif()
