-- Independent script targets, making it easy to modify the script at runtime and re-run independent builds
target("scripts")
    set_kind("phony")
    set_default(get_config("scripting"))
    -- The jspp package provides scriptc and the runtime library; scriptc links
    -- the same quickjs build as the runtime, keeping compiled bytecode loadable.
    add_packages("jspp")

    on_load(function (target)
        import("core.project.config")
        -- register cleanfiles (tsc outdir and scriptc outdir)
        local dist_top = path.join(config.builddir(), ".dist")
        target:add("cleanfiles", path.absolute(dist_top, os.projectdir()))
        local qjsc_top = path.join(config.builddir(), ".scriptc")
        target:add("cleanfiles", path.absolute(qjsc_top, os.projectdir()))
    end)

    on_build(function (target, opt)
        import("lib.detect.find_tool")
        import("core.project.depend")
        import("utils.progress")
        import("core.project.config")

        local tsc = assert(find_tool("tsc.cmd", { shell = true }), "tsc not found")

        -- 1) depend that runs tsc when TS sources change

        local tsconfig_files = {}
        local root_tsconfig = path.join(os.projectdir(), "tsconfig.json")
        if os.isfile(root_tsconfig) then
            table.insert(tsconfig_files, root_tsconfig)
        end

        local sourcefiles = os.files("src-scripts/**.ts")
        local tsc_inputs = table.join(sourcefiles, tsconfig_files)

        local tsc_outdir = path.absolute(path.join(config.builddir(), ".dist"), os.projectdir())

        local tsc_stamp = path.join(tsc_outdir, ".tsc.stamp") -- guard file
        table.insert(tsc_inputs, tsc_stamp)

        depend.on_changed(function ()
            os.mkdir(tsc_outdir)
            progress.show(opt.progress, "${color.build.target}build.tsc %s", "src-scripts")
            os.vrunv(tsc.program, {}, { shell = true })
            io.writefile(tsc_stamp, "ok\n")
        end, {
            files = tsc_inputs,
            dependfile = target:dependfile(tsc_stamp),
            changed = target:is_rebuilt()
        })

        -- 2) compile the tsc output to quickjs bytecode with scriptc from the
        --    jspp package. scriptc links the same quickjs build as the runtime
        --    (bytecode embeds a build-dependent checksum) and emits one blob
        --    per entry; imports stay runtime-resolved specifiers.

        -- collect native module names declared in the d.ts files; passed via
        -- -M so scriptc does not treat them as file imports
        local scan_files = os.files("src-scripts/types/**.d.ts") or {}
        local scan_seen, ignore_module_args = {}, {}
        local scan_pattern = "^%s*declare%s+module%s+['\"](.-)['\"]"
        for _, f in ipairs(scan_files) do
            local content = io.readfile(f)
            if content then
                for line in content:gmatch("[^\r\n]+") do
                    local name = line:match(scan_pattern)
                    if name and not scan_seen[name] then
                        scan_seen[name] = true
                        table.insert(ignore_module_args, "-M")
                        table.insert(ignore_module_args, name)
                    end
                end
            end
        end

        -- resolve scriptc: the jspp package adds its bin directory to PATH;
        -- fall back to the package install directory
        local scriptc = find_tool("scriptc", { norun = true })
        if not scriptc then
            local pkg = target:pkg("jspp")
            if pkg then
                local exe = path.join(pkg:installdir(), "bin", is_plat("windows") and "scriptc.exe" or "scriptc")
                if os.isfile(exe) then
                    scriptc = { program = exe }
                end
            end
        end
        if not scriptc then
            raise("scriptc not found: the jspp package does not provide the bytecode compiler; update the jspp package")
        end

        local scriptc_outdir = path.absolute(path.join(config.builddir(), ".scriptc"), os.projectdir());
        os.mkdir(scriptc_outdir)

        -- resolve entry file, scriptc compi
        local entry_fname = "index.js"
        local entry_file = path.join(tsc_outdir, entry_fname)
        if not os.isfile(entry_file) then
            raise("entry file not found: %s", entry_file)
        end

        local outfile = path.join(scriptc_outdir, "index.bin")
        depend.on_changed(function ()
            local rel_js = path.relative(entry_file, os.projectdir()) or entry_file
            progress.show(opt.progress, "${color.build.target}build.scriptc %s", rel_js)

            -- scriptc runs with cwd = tsc_outdir: module names are baked
            -- relative to it via -n, so the runtime anchors them at the
            -- deployed scripts directory. -m forces module mode: the tsc
            -- output is always an ES module, and JS_DetectModule may
            -- misclassify relative imports.
            local args = table.join({"-m"}, ignore_module_args, {"-n", entry_fname, "-o", outfile, entry_file})
            os.vrunv(scriptc.program, args)
        end, {
            files = { entry_file },
            lastmtime = os.mtime(outfile),
            changed = target:is_rebuilt()
        })
    end)

    --copy build/.scriptc into bin/PLand/scripts
    after_build(function(target)
        local projectdir = os.projectdir()
        local bindir = path.join(projectdir, "bin")
        local outputdir = path.join(bindir, "PLand", "scripts")

        os.mkdir(outputdir)

        local builddir = path.join(projectdir, "build")
        local qjscdir = path.join(builddir, ".scriptc/*")
        os.cp(qjscdir, outputdir)
    end)
target_end()
