use 5.008008;
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME              => 'Data::HexConverter',
    VERSION_FROM      => 'lib/Data/HexConverter.pm',
    PREREQ_PM         => {},
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/Data/HexConverter.pm',
       AUTHOR         => 'A. U. Thor <author@example.com>') : ()),
    CONFIGURE_REQUIRES => {
        'Devel::PPPort' => '3.19',
    },
    LIBS              => [''],
    DEFINE            => '',
    INC               => '-I.',
    C                 => ['HexConverter.c', 'hexsimd.c'],
    OBJECT            => '$(O_FILES)',
    CCFLAGS           => '-O3 -Wall -Wextra -fPIC -fvisibility=hidden -march=native',
    depend            => { '$(O_FILES)' => 'ppport.h' },
);

sub MY::postamble {
    return <<'EOF';
ppport.h:
	$(PERL) -MDevel::PPPort -e "Devel::PPPort::WriteFile('ppport.h')"
EOF
}
