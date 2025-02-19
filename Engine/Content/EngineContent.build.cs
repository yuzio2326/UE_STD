// Copyright (c) Ubisoft. All Rights Reserved.
// Licensed under the Apache 2.0 License. See LICENSE.md in the project root for license information.
using System;
using System.IO;
using Sharpmake;

[Generate]
public class EngineContent : CommonProject
{
    public EngineContent() 
    {
        SourceFilesExtensions.Add(".fbx");
        SourceFilesExtensions.Add(".dds");
        SourceFilesExtensions.Add(".png");
    }

    public override void ConfigureAll(Configuration conf, EngineTarget target)
    {
        base.ConfigureAll(conf, target);
        conf.Output = Configuration.OutputType.Utility;

        conf.TargetPath += "/Content";

        conf.EventPreBuild.Clear();
        conf.EventPreBuild.Add(@"(robocopy /s ""[project.SourceRootPath]"" " + @"""" + conf.TargetPath + @"""" + " *.fbx *.dds *.png /njh /njs /ndl /nc /ns) ^& IF %ERRORLEVEL% LEQ 3 exit 0");
    }
}