# source this script in your configure

tconf_print () {
	echo "$@" 1>&2
}

tconf_help () {
	echo "usage : $(basename "$0") [OPTIONS..]
generate config.mk from environement
--cc=CC set the C compiler
--cflags=CFLAGS set the CFLAGS
--prefix=PREFIX set the prefix"
}

tconf_init () {
	TOP="$(realpath "$(dirname $0)")"
	OPT=""
	TCONF_DIR="$TOP/tconf"
	mkdir -p "$TCONF_DIR"

	# defaults
	if [ -z "$PREFIX" ] ; then
		PREFIX="/usr/local"
	fi

	if [ -z "$CFLAGS" ] ; then
		CFLAGS="-Wall -Wextra"
	fi

	# parse options
	for i in "$@" ; do
		case "$i" in
		--cc=*|CC=*)
			CC="${i#*=}"
			;;
		--as=*|AS=*)
			AS="${i#*=}"
			;;
		--ar=*|AR=*)
			AR="${i#*=}"
			;;
		--ld=*|LD=*)
			LD="${i#*=}"
			;;
		--prefix=*|PREFIX=*)
			PREFIX="${i#*=}"
			;;
		--cflags=*|CFLAGS=*)
			CFLAGS="${i#*=}"
			;;
		--ldflags=*|LDFLAGS=*)
			LDFLAGS="${i#*=}"
			;;
		--asflags=*|ASFLAGS=*)
			ASFLAGS="${i#*=}"
			;;
		--host=*|HOST=*)
			HOST="${i#*=}"
			;;
		--build=*|BUILD=*)
			BUILD="${i#*=}"
			;;
		--debug)
			OPT="$OPT -DDEBUG=1"
			;;
		--help)
			tconf_help
			exit 0
			;;
		--*)
			tconf_print "unknow option '$i' (see --help)"
			exit 1
			;;
		esac
	done
}

tconf_echo_conf () {
	if [ $# != 2 ] ; then
		tconf_print "usage : tconf_echo_conf NAME VAR"
		return 1
	fi
	test -n "$2" && echo "$1=$2"
}

tconf_fini () {
	{
		echo "# automaticly generated from $(basename "$0")"
		echo "PREFIX=$PREFIX"
		echo "CFLAGS=$CFLAGS $OPT"
		tconf_echo_conf CC "$CC"
		tconf_echo_conf AS "$AS"
		tconf_echo_conf AR "$AR"
		tconf_echo_conf LD "$LD"
		tconf_echo_conf HOST "$HOST"
		tconf_echo_conf ARCH "$ARCH"
	} > "$TOP/config.mk"
}

tconf_uppercase () {
	echo "$@" | tr a-z./ A-Z__
}

tconf_require () {
	if [ $# != 2 ] ; then
		tconf_print "usage : tconf_require NAME VAR"
		return 1
	fi
	if test -z "$2" ; then
		tconf_print "no $1 found"
		exit 1
	fi
}

tconf_check_code () {
	if [ $# != 3 ] ; then
		tconf_print "usage : tconf_check_code CC NAME CODE"
		return 1
	fi

	FILE="$TCONF_DIR/check-$2.c"
	mkdir -p "$(dirname "$FILE")"

	tconf_print -n "check $2... "
	
	# check if we aready checked this
	if test -f "$FILE.out" ; then
		tconf_print "yes(cached)"
		OPT="$OPT -DHAVE_$(tconf_uppercase "$2")=1"
		return 0
	fi

	echo "$3" > "$FILE"
	if $1 "$FILE" -o "$FILE.out" >/dev/null 2>/dev/null ; then
		tconf_print "yes"
		OPT="$OPT -DHAVE_$(tconf_uppercase "$2")=1"
		return 0
	else
		tconf_print "no"
		return 1
	fi
}

tconf_check_func () {
	if [ $# != 3 ] ; then
		tconf_print "usage : tconf_check_func CC HEADER FUNC"
		return 1
	fi
	tconf_check_code "$1" "$3" "#include <$2>
void *volatile ptr = (void*)$3;
int main() {
	return 0;
}"
}

tconf_check_header () {
	if [ $# != 2 ] ; then
		tconf_print "usage : tconf_check_header CC HEADER"
		return 1
	fi

	tconf_check_code $1 "$2" "#include <$2>
int main () {
	return 0;
}"
}

tconf_require_header () {
	if [ $# != 2 ] ; then
		tconf_print "usage : tconf_require_header CC HEADER"
		return 1
	fi
	if ! tconf_check_header "$1" "$2" ; then
		tconf_print "$2 is required"
		exit 1
	fi
}

tconf_search_util () {
	if test -z "$1" ; then
		tconf_print "usage : tconf_search_util NAME PREFIX UTILS..."
		return 1
	fi

	UTIL_PREFIX="$2"
	test -n "$UTIL_PREFIX" && UTIL_PREFIX="$UTIL_PREFIX-"
	tconf_print -n "search $1... "

	# skip arg and prefix
	shift 2
	for util in "$@" ; do
		if ${UTIL_PREFIX}$util --version 2>/dev/null >/dev/null ; then
			tconf_print "${UTIL_PREFIX}$util"
			echo "${UTIL_PREFIX}$util"
			return 0
		fi
	done
	tconf_print "no"
	return 1
}

tconf_search_cc () {
	if [ $# != 1 ] ; then
		tconf_print "usage : tconf_search_cc PREFIX"
		return 1
	fi
	if test -n "$CC" ; then
		tconf_print "check C compiler... $CC"
		return 0
	fi
	CC="$(tconf_search_util "C compiler" "$1" gcc clang tcc cc)"
}

tconf_find_build () {
	tconf_print -n "build os... "
	if test -n "$BUILD" ; then
		tconf_print "$BUILD"
		return 0
	fi
	if test -n "$CC_FOR_BUILD" && BUILD="$($CC_FOR_BUILD -dumpmachine )" ; then
		tconf_print "$BUILD"
		return 0
	fi
	tconf_print "unknow"
	return 1
}

tconf_find_host () {
	tconf_print -n "host os... "
	if test -n "$HOST" ; then
		tconf_print "$HOST"
		return 0
	fi
	if test -n "$CC" && HOST="$($CC -dumpmachine)" ; then
		tconf_print "$HOST"
		return 0
	fi
	tconf_print "unknow"
	return 1
}

tconf_find_os () {
	tconf_find_build
	tconf_find_host
}
