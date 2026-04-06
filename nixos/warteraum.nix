{ config, lib, pkgs, ... }:

with lib;

let
  cfg = config.services.warteraum;

  flipdot-gschichtler = import ../. {
    inherit pkgs;
  };

  userGroupName = "warteraum";
in {
  options = {
    services.warteraum = {
      enable = mkEnableOption "warteraum";

      package = mkOption {
        type = types.package;
        default = flipdot-gschichtler.warteraum;
        defaultText = literalExample "(import ../. { inherit pkgs; }).warteraum";
        description = ''
          <literal>warteraum</literal> derivation to use.
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

      serviceConfig = {
        Type = "simple";

        ExecStart = lib.concatStringsSep " " [
          # See https://github.com/systemd/systemd/issues/22549
          "${pkgs.coreutils}/bin/env"
          "WARTERAUM_SALT_FILE=\${CREDENTIALS_DIRECTORY}/salt"
          "WARTERAUM_TOKENS_FILE=\${CREDENTIALS_DIRECTORY}/tokens"
          "${lib.getExe' cfg.package "warteraum"}"
        ];

        LoadCredential = [
          "salt:${cfg.saltFile}"
          "tokens:${cfg.tokensFile}"
        ];

        # make sure only /nix/store is accessible
        TemporaryFileSystem = "/:ro";
        BindReadOnlyPaths = "${builtins.storeDir} -/etc/ld-nix.so.preload";
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

    users = {
      users."${userGroupName}"= {
        isSystemUser = true;
        group = userGroupName;
      };
      groups."${userGroupName}"= { };
    };
  };
}
