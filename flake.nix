# Run with:
# $ nix run '.?submodules=1'
# Or build with:
# $ nix build '.?submodules=1'

{
  description = "TS Spect Compiler";

  inputs = {
    nixpkgs.url      = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url  = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ ];
        pkgs = import nixpkgs {
          inherit system overlays;
        };
        buildSpect = pname: (with pkgs; stdenv.mkDerivation {
          pname = "${pname}";
          # FIXME how are the versions determined?
          version = "0.1";
          src = self;
          nativeBuildInputs = [
            cmake
            clang
            python310
            libxslt
          ] ++ (with pkgs.python310Packages; [
            jinja2
          ]);
          # buildPhase = "bash ../build.sh --clean";
          configurePhase = ''
            python generate_cover_class.py --input templates/InstructionDefs_v2.txt --output src/cosim/spect_instr_gen_coverage.svh --cov-template templates/coverage_class_template.txt --instr-defs templates/InstructionDefs_v2.txt
            mkdir --parents build
            pushd build
            cmake ..
            '';

          buildPhase = ''
            make
          '';

          installPhase = ''
            mkdir -p $out/bin
            mv $TMP/source/build/src/apps/spect_compiler $out/bin
            mv $TMP/source/build/src/apps/spect_iss $out/bin
          '';
        }
      );
      in
      with pkgs;
      {
        packages.spect-compiler = buildSpect "spect_compiler";
        packages.spect-iss = buildSpect "spect_iss";
        defaultPackage = buildSpect "spect_compiler";
        devShells.default = mkShell {
          buildInputs = [
            cmake
            clang
            python310
            libxslt
          ] ++ (with pkgs.python310Packages; [
            jinja2
          ]);
        };
      }
    );
}
