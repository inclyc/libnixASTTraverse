# save this as shell.nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.callPackage ./default.nix { }
