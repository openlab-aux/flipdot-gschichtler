{ config, lib, pkgs, flipdot-gschichtler, ... }:

with lib;

let
  cfg = config.services.flipdot-gschichtler;
  inherit (flipdot-gschichtler)
    bahnhofshalle
    warteraum-static
    ;
in {
  options = {
    services.flipdot-gschichtler = {
      enable = mkEnableOption "flipdot-gschichtler";

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
        ExecStart = "${warteraum-static}/bin/warteraum";
        InAccessibleDirectories = "/";
        # mmap and munmap are used by libscrypt-kdf
        SystemCallFilter = "@default @basic-io @io-event @network-io fcntl @signal @process @timer brk mmap munmap open";
        SystemCallArchitectures = "native";
        CapabilityBoundingSet = "";

        NoNewPrivileges = true;
        RestrictRealtime = true;
        LockPersonality = true;

        DynamicUser = true;

        ProtectSystem = "strict";
        ProtectHome = true;
        PrivateTmp = true;
        PrivateUsers = true;
        ProtectKernelTunables = true;
        ProtectKernelModules = true;
        ProtectControlGroups = true;
        ProtectKernelLogs = true;
        MemoryDenyWriteExecute = true;
        PrivateDevices = true;
        PrivateMounts = true;
      };
    };

    services.nginx.virtualHosts."${cfg.virtualHost}" = {
      enableACME = true;
      forceSSL = true;
      root = bahnhofshalle;
      extraConfig = ''
        location /api {
          proxy_pass http://127.0.0.1:9000/api;
        }
      '';
    };
  };
}
