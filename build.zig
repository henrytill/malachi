const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const config_lib = b.addStaticLibrary(.{
        .name = "libconfig",
        .target = target,
        .optimize = optimize,
    });
    const config_lib_include = "include/config";
    const config_lib_sources = &.{
        "lib/config/config.c",
    };

    config_lib.linkLibC();
    config_lib.addIncludePath(.{ .path = config_lib_include });
    if (@hasDecl(std.Build.Step.Compile, "AddCSourceFilesOptions")) {
        config_lib.addCSourceFiles(.{ .files = config_lib_sources });
    } else {
        config_lib.addCSourceFiles(config_lib_sources, &.{});
    }

    const main_exe = b.addExecutable(.{
        .name = "malachi",
        .root_source_file = .{ .path = "bin/main.c" },
        .target = target,
        .optimize = optimize,
    });

    main_exe.linkLibC();
    main_exe.linkSystemLibrary("mupdf");
    main_exe.linkSystemLibrary("sqlite3");
    main_exe.linkLibrary(config_lib);
    main_exe.addIncludePath(.{ .path = config_lib_include });
    b.installArtifact(main_exe);

    const config_test_exe = b.addExecutable(.{
        .name = "config_test",
        .root_source_file = .{ .path = "test/config_test.c" },
        .target = target,
        .optimize = optimize,
    });

    config_test_exe.linkLibC();
    config_test_exe.linkLibrary(config_lib);
    config_test_exe.addIncludePath(.{ .path = config_lib_include });
    b.installArtifact(config_test_exe);

    const run_config_test = b.addRunArtifact(config_test_exe);
    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_config_test.step);
}
