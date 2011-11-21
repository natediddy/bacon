# Update bacon's version string in certain files

if [ $# != 3 ]; then
  echo "usage: $0 \$1 \$2 \$3"
  echo "  where:"
  echo "    '\$1' = major release integer,"
  echo "    '\$2' = minor release integer,"
  echo "    '\$3' = build version integer"
  exit
fi

sed -i "s/bacon, [0-9].[0-9][0-9].[0-9]/bacon, $1.$2.$3/" \
  configure.ac
sed -i "s/pkgver=[0-9].[0-9][0-9].[0-9]/pkgver=$1.$2.$3/" linux/arch/PKGBUILD
sed -i "s/version [0-9].[0-9][0-9].[0-9]/version $1.$2.$3/" docs/bacon.1
