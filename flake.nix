{
  description = "gKiosk - minimal kiosk web browser and bootable NixOS image";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
    gkiosk = pkgs.callPackage ./default.nix {};
  in {
    packages.${system} = {
      default = gkiosk;
      gkiosk = gkiosk;

      iso = self.nixosConfigurations.kiosk.config.system.build.isoImage;
    };

    nixosConfigurations.kiosk = nixpkgs.lib.nixosSystem {
      inherit system;
      modules = [
        ({ config, pkgs, lib, ... }: {
          system.stateVersion = "24.11";

          boot.loader.systemd-boot.enable = true;
          boot.loader.efi.canTouchEfiVariables = true;
          boot.kernelParams = [ "quiet" "splash" ];

          networking.hostName = "gkiosk";
          networking.networkmanager.enable = true;

          hardware.graphics.enable = true;

          # Cage: minimal Wayland compositor that runs a single app fullscreen
          services.cage = {
            enable = true;
            user = "kiosk";
            program = "${gkiosk}/bin/gkiosk";
            environment = {
              KIOSK_URL = "https://example.com/";
            };
          };

          # Persistent cookie storage
          systemd.tmpfiles.rules = [
            "d /var/lib/gkiosk 0700 kiosk kiosk -"
          ];

          environment.variables = {
            XDG_DATA_HOME = "/var/lib/gkiosk";
          };

          users.users.kiosk = {
            isSystemUser = true;
            group = "kiosk";
            home = "/var/lib/gkiosk";
          };
          users.groups.kiosk = {};

          # Xbox controller support
          hardware.xpadneo.enable = true;
          boot.kernelModules = [ "xpad" "joydev" ];
          services.udev.extraRules = ''
            SUBSYSTEM=="input", ATTRS{idVendor}=="045e", MODE="0666"
          '';

          environment.systemPackages = [ gkiosk ];

          # Minimal system — no docs, no extra services
          documentation.enable = false;
          services.openssh.enable = false;

          # Auto-login, no display manager
          services.getty.autologinUser = "kiosk";

          # Build ISO image
          isoImage = {
            isoName = "gkiosk.iso";
            makeEfiBootable = true;
            makeUsbBootable = true;
          };
        })

        "${nixpkgs}/nixos/modules/installer/cd-dvd/iso-image.nix"
        "${nixpkgs}/nixos/modules/profiles/minimal.nix"
      ];
    };
  };
}
