#Change the variable below acording your apache modules path
%define moduledir /usr/local/apache/modules

Name: Apache-Get-Video-Thumbnail
Version: 1.0.0
Release: 1
Summary: Video Thumb Extractor

Group: System Environment/Daemons
License: GPL
URL: https://github.com/dayvson/Apache-Get-Video-Thumbnail
#Source0: https://github.com/dayvson/Apache-Get-Video-Thumbnail/zipball/master
#Source0: https://github.com/dayvson/Apache-Get-Video-Thumbnail/zipball/Apache-Get-Video-Thumbnail-1.0.0-1.tar.gz
Source0: https://github.com/dayvson/Apache-Get-Video-Thumbnail/zipball/Apache-Get-Video-Thumbnail-%{version}-%{release}.tar.gz
#BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-root)
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: apr-devel
BuildRequires: ffmpeg-devel >= 0.11.1-4
BuildRequires: imlib2-devel
BuildRequires: libdc1394-devel
BuildRequires: libogg-devel
BuildRequires: libtheora-devel
BuildRequires: SDL-devel
BuildRequires: x264-devel >= 0.120-5
Requires: ffmpeg >= 0.11.1-4
Provides: mod_videothumb

%description
Video Thumb Extractor is a module for Apache2 to extract an image from a video frame from a specific
second resizing/cropping it to a given size and to extract a storyboard(multiple video frames on sprite) from video.

%prep
%setup -q


%build
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%clean
rm -rf %{buildroot}


%files
%defattr(-,root,root,0444)
/usr/local/apache/modules/mod_videothumb.so
%doc



%changelog
* Tue Sep 10 2012 Alexandre Bunn <alexandre.bunn@corp.terra.com.br> - 0.4.0-1
- Build rpm package for Centos 6 x86_64

* Tue Aug 21 2012 Alexandre Bunn <alexandre.bunn@corp.terra.com.br> - 0.3.0-1
- Build rpm package for Centos 6 x86_64
