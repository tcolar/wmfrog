# $Header: /home/cvsroot/gentoo-x86/x11-plugins/wmfrog/wmfrog-0.1.6.ebuild

DESCRIPTION="Dockable graphical weather application for Windowmaker/Fluxbox, etc ..."
SRC_URI="http://www.colar.net/wmapps/${P}.tgz"
HOMEPAGE="http://www.colar.net/wmapps/"

DEPEND="virtual/x11
	dev-lang/perl"

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="x86 sparc amd64"

src_compile() {
	cd ${S}/Src
	emake || die
}

src_install () {
	dodoc CHANGES COPYING README
	cd ${S}/Src
	make DESTDIR=${D} install || die
}
