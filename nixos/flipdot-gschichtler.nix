{ config, lib, pkgs, ... }:

with lib;

let
  cfg = config.services.flipdot-gschichtler;

  flipdot-gschichtler = import ../. {
    inherit pkgs;
  };
  importClause = "(import ../. { inherit pkgs; })";

  userGroupName = "warteraum";
in {
  options = {
    services.flipdot-gschichtler = {
      enable = mkEnableOption "flipdot-gschichtler";

      packages = {
        warteraum = mkOption {
          type = types.package;
          default = flipdot-gschichtler.warteraum;
          defaultText = literalExample "${importClause}.warteraum";
          description = ''
            <literal>warteraum</literal> derivation to use.
          '';
        };
        bahnhofshalle = mkOption {
          type = types.package;
          default = flipdot-gschichtler.bahnhofshalle;
          defaultText = literalExample "${importClause}.bahnhofshalle";
          description = ''
            <literal>bahnhofshalle</literal> derivation to use.
          '';
        };
      };

      virtualHost = mkOption {
        type = types.str;
        default = "localhost";
        description = ''
          Virtual Host for nginx to use for serving
          warteraum and bahnhofshalle.
        '';
      };

      saltFile = mkOption {
        type = types.str;
        description = ''
          File of random data to use as salt for storing
          API tokens. Using a path here will copy secrets
          into the nix store!
        '';
      };

      tokensFile = mkOption {
        type = types.path;
        description = ''
          File containing authorized API tokens which
          can be created using
          <literal>''${warteraum}/bin/hashtoken</literal>.
          Using a path here will copy secrets into the
          nix store!
        '';
      };
    };
  };

  config = mkIf cfg.enable {
    systemd.services.warteraum = {
      description = "Warteraum REST API http server of flipdot-gschichtler";
      after = [ "network.target" ];
      wantedBy = [ "multi-user.target" ];

      environment = {
        WARTERAUM_SALT_FILE = cfg.saltFile;
        WARTERAUM_TOKENS_FILE = cfg.tokensFile;
      };

      serviceConfig = {
        Type = "simple";
        ExecStart = "${cfg.packages.warteraum}/bin/warteraum";

        # make only /nix/store and the salt and token file accessible
        TemporaryFileSystem = "/:ro";
        BindReadOnlyPaths = "/nix/store " + (lib.concatStringsSep " " [
          cfg.saltFile
          cfg.tokensFile
        ]);
        # TemporaryFileSystem doesn't work with DynamicUser
        User = userGroupName;
        Group = userGroupName;

        # mmap and munmap are used by libscrypt-kdf
        SystemCallFilter = lib.concatStringsSep " " [
          "@default"
          "@basic-io"
          "@io-event"
          "@network-io"
          "fcntl"
          "@signal"
          "@process"
          "@timer"
          "brk"
          "mmap" "munmap" "mprotect"
          "@file-system"
        ];
        SystemCallArchitectures = "native";

        CapabilityBoundingSet = "";
        NoNewPrivileges = true;
        RestrictRealtime = true;
        LockPersonality = true;

        PrivateUsers = true;
        ProtectKernelTunables = true;
        ProtectKernelModules = true;
        ProtectControlGroups = true;
        ProtectKernelLogs = true;
        MemoryDenyWriteExecute = true;
        PrivateDevices = true;
        PrivateMounts = true;

        StandardError = "journal";
        StandardOutput = "journal";
      };
    };

    services.nginx.virtualHosts."${cfg.virtualHost}" = {
      enableACME = true;
      forceSSL = true;
      root = cfg.packages.bahnhofshalle;
      extraConfig = ''
        location /api {
          proxy_pass http://127.0.0.1:9000/api;
        }
      '';
    };

    users = {
      users."${userGroupName}"= {
        isSystemUser = true;
        group = userGroupName;
      };
      groups."${userGroupName}"= {};
    };
  };
}
