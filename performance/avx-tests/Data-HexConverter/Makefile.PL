use 5.008008;
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME              => 'Data::HexConverter',
    VERSION_FROM      => 'lib/Data/HexConverter.pm',
    PREREQ_PM         => { 'Test::More' => 0 },
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/Data/HexConverter.pm',
       AUTHOR         => 'A. U. Thor <author@example.com>') : ()),
    CONFIGURE_REQUIRES => {
        'Devel::PPPort' => '3.19',
    },
    LIBS              => ['-L. -lhexsimd'], # Link against our library
    DEFINE            => '',
    INC               => '-I.',
    C                 => ['HexConverter.c'], # Only compile the XS file
    OBJECT            => 'HexConverter.o',
    depend            => { '$(O_FILES)' => 'ppport.h' },
    clean             => { FILES => 'libhexsimd.so hexsimd.o' },
);

# Custom section to build the shared library and ensure it's linked
sub MY::postamble {
    return <<'END_MAKE_FRAGMENT';
all :: libhexsimd.so

libhexsimd.so: hexsimd.o
	$(CC) -shared -o $@ $^

hexsimd.o:
	$(CC) -c $(CFLAGS) -O3 -Wall -Wextra -fPIC -fvisibility=hidden -march=native hexsimd.c -o $@

ppport.h:
	$(PERL) -MDevel::PPPort -e "Devel::PPPort::WriteFile('ppport.h')"

# Add rpath to the XS library so it can find libhexsimd.so
LD_RUN_PATH := $(LD_RUN_PATH):$(CURDIR)

END_MAKE_FRAGMENT
}

