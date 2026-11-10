{
	description = "Sheet music viewer with Barbershop tag support";
	inputs = {
		quartz.url = "github:vimpostor/quartz";
	};

	outputs = { self, quartz }: quartz.lib.eachSystem (system:
		let
			pkgs = quartz.inputs.nixpkgs.legacyPackages.${system};
			stdenvs = [ { name = "gcc"; pkg = pkgs.gccStdenv; } { name = "clang"; pkg = pkgs.llvmPackages.stdenv; } ];
			defaultStdenv = (builtins.head stdenvs).name;
			fftune = pkgs.fetchFromGitHub {
				owner = "fftune";
				repo = "fftune";
				rev = "aceedd5272c462820ecae2f772038b38b55bc13a";
				hash = "sha256-diXPby4v5wcANdZWoHpyLz/HXGBURW+VGKfNfkPgAWY=";
			};
			makeStdenvPkg = env: env.mkDerivation {
				pname = "tagalong";
				version = quartz.lib.cmakeProjectVersion ./CMakeLists.txt;

				src = ./.;

				nativeBuildInputs = with pkgs; [
					cmake
					pkg-config
					qt6.wrapQtAppsHook
					imagemagick
				];
				buildInputs = with pkgs; [
					qt6.qtbase
					qt6.qtdeclarative
					qt6.qtmultimedia
					qt6.qtwebengine
					qt6.qtsvg
					fftwFloat
					libsmf
					libsndfile
					glib
				];

				cmakeFlags = quartz.lib.cmakeWrapper { inherit pkgs; cmakeFile = ./CMakeLists.txt; } ++ [("-DFETCHCONTENT_SOURCE_DIR_FFTUNE=" + fftune)];
				buildFlags = "icon-integration all";
			};
		in {
			packages = {
				default = self.outputs.packages.${system}.${defaultStdenv};
			} // builtins.listToAttrs (map (x: { name = x.name; value = makeStdenvPkg x.pkg; }) stdenvs);
			checks = {
				format = pkgs.runCommand "format" { src = ./.; nativeBuildInputs = [ pkgs.clang-tools pkgs.git ]; } "mkdir $out && cd $src && find . -type f -path './*\\.[hc]pp' -exec clang-format -style=file --dry-run --Werror {} \\;";
			};
		}
	);
}
