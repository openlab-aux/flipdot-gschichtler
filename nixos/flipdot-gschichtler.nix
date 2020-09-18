{ config, lib, pkgs, flipdot-gschichtler, ... }:

with lib;

let
  cfg = config.services.flipdot-gschichtler;
  fg  = flipdot-gschichtler;
  withTokens = fg.warteraum-static.override {
    inherit (cfg) apiTokens;
    scryptSalt = cfg.salt;
  };
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

      salt = mkOption {
        type = types.str;
        description = ''
          Salt to use for hashing API tokens using scrypt_kdf(3).
          Must be a string of hexadecimals which has a multiple of
          2 as a length.
        '';
      };

      apiTokens = mkOption {
        type = types.listOf types.str;
        default = [];
        description = ''
          List of API tokens to allow access.
          May be strings of any length.
        '';
      };
    };
  };

  config = mkIf cfg.enable {
    systemd.services.warteraum = {
      description = "Warteraum REST API http server of flipdot-gschichtler";
      after = [ "network.target" ];
      wantedBy = [ "multi-user.target" ];

      serviceConfig = {
        Type = "simple";
        ExecStart = "${withTokens}/bin/warteraum";
        InAccessibleDirectories = "/";
        # mmap and munmap are used by libscrypt-kdf
        SystemCallFilter = "@default @basic-io @io-event @network-io fcntl @signal @process @timer brk mmap munmap";
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
      root = fg.bahnhofshalle;
      extraConfig = ''
        location /api {
          proxy_pass http://127.0.0.1:9000/api;
        }
      '';
    };
  };
}
