﻿// Copyright (c) Ubisoft. All Rights Reserved.
// Licensed under the Apache 2.0 License. See LICENSE.md in the project root for license information.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;
using Sharpmake.Generators.FastBuild;

[module: Sharpmake.Include("HelloAndroid.*.sharpmake.cs")]
[module: Sharpmake.Include("codebase/*.sharpmake.cs")]
[module: Sharpmake.Include("codebase/*/*.sharpmake.cs")]

namespace HelloAndroid
{
    public static class Globals
    {
        // branch root path relative to current sharpmake file location
        public const string RelativeRootPath = @".\codebase";
        public static string RootDirectory;
        public static string TmpDirectory { get { return Path.Combine(RootDirectory, "temp"); } }
        public static string OutputDirectory { get { return Path.Combine(TmpDirectory, "bin"); } }

        public static Dictionary<DevEnv, string> AndroidTargetsPathForDevEnv = new Dictionary<DevEnv, string>();
    }

    public static class AndroidUtil
    {
        public static void SetupDefaultSDKPaths()
        {
            string AndroidSdkPath = System.Environment.GetEnvironmentVariable("ANDROID_SDK_ROOT");
            string AndroidNdkPath = System.Environment.GetEnvironmentVariable("ANDROID_NDK_ROOT");
            string JavaPath = System.Environment.GetEnvironmentVariable("JAVA_HOME");
            string AntPath = System.Environment.GetEnvironmentVariable("ANT_HOME");

            if (AndroidSdkPath == null)
            {
                throw new Error("ANDROID_SDK_ROOT environment variable undefined");
            }
            else if (!Directory.Exists(AndroidSdkPath))
            {
                throw new Error(AndroidSdkPath + " directory does not exist.");
            }

            if (AndroidNdkPath == null)
            {
                throw new Error("ANDROID_NDK_ROOT environment variable undefined");
            }
            else if (!Directory.Exists(AndroidNdkPath))
            {
                throw new Error(AndroidNdkPath + " directory does not exist.");
            }

            if (JavaPath == null)
            {
                throw new Error("JAVA_HOME environment variable undefined");
            }

            if (AntPath == null)
            {
                throw new Error("ANT_HOME environment variable undefined");
            }

            // Android global settings config
            {
                Android.GlobalSettings.AndroidHome = AndroidSdkPath;
                Android.GlobalSettings.JavaHome = JavaPath;
                Android.GlobalSettings.AntHome = AntPath;
                Android.GlobalSettings.NdkRoot = AndroidNdkPath;
            }
        }
    }

    public static class Main
    {
        private static void ConfigureRootDirectory()
        {
            FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
            string rootDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeRootPath);
            Globals.RootDirectory = Util.SimplifyPath(rootDirectory);
        }

        private static void ConfigureAutoCleanup()
        {
            Util.FilesAutoCleanupActive = true;
            Util.FilesAutoCleanupDBPath = Path.Combine(Globals.TmpDirectory, "sharpmake");

            if (!Directory.Exists(Util.FilesAutoCleanupDBPath))
                Directory.CreateDirectory(Util.FilesAutoCleanupDBPath);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            AndroidUtil.SetupDefaultSDKPaths();

            ConfigureRootDirectory();
            ConfigureAutoCleanup();

            FastBuildSettings.FastBuildWait = true;
            FastBuildSettings.FastBuildSummary = false;
            FastBuildSettings.FastBuildNoSummaryOnError = true;
            FastBuildSettings.FastBuildDistribution = false;
            FastBuildSettings.FastBuildMonitor = true;
            FastBuildSettings.FastBuildAllowDBMigration = true;

            // for the purpose of this sample, we'll reuse the FastBuild executables that live in the sharpmake source repo
            string sharpmakeFastBuildDir = Util.PathGetAbsolute(Globals.RootDirectory, @"..\..\..\tools\FastBuild");
            switch (Util.GetExecutingPlatform())
            {
                case Platform.linux:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(sharpmakeFastBuildDir, "Linux-x64", "fbuild");
                    break;
                case Platform.mac:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(sharpmakeFastBuildDir, "OSX-x64", "FBuild");
                    break;
                case Platform.win64:
                default:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(sharpmakeFastBuildDir, "Windows-x64", "FBuild.exe");
                    break;
            }

            Bff.UnityResolver = new Bff.FragmentUnityResolver();

            foreach (Type solutionType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonSolution))))
                arguments.Generate(solutionType);
        }
    }
}
