/** --------------------------------------------------------------------
 **
 **  Define the simulation setup for n_TOF
 **  Author: <j.l.rodriguez.sanchez@udc.es>
 **
 **  Last Update: 10/04/26
 **  Comments:
 **         - 10/04/26 : Initial setup
 **
 **  Configuration:
 **  (1) Select the right generator "fGenerator"
 **  (2) Select the detectors that you wish for the simulation, for instance,
 **
 **  Execute it as follows:
 **  root -l 'runsim.C(1000)'
 **  where 1000 means the number of events
 **
 **/

void runsim(Int_t nEvents = 1)
{
    // ----------- Configuration area ----------------------------------

    TString OutFile = "sim.root"; // Output file for data
    TString ParFile = "par.root"; // Output file for params

    FairLogger::GetLogger()->SetLogScreenLevel("info");
    FairLogger::GetLogger()->SetColoredLog(true);
    FairLogger::GetLogger()->SetLogVerbosityLevel("low");

    Bool_t fVis = true; // Store tracks for visualization

    // MonteCarlo engine: TGeant3, TGeant4, TFluka  --------------------
    TString fMC = "TGeant4";

    // Event generator type: box for particles or inclroot
    TString generator1 = "box";
    TString generator2 = "inclroot";
    TString fGenerator = generator2;

    TString fPpacsGeo = "target_area_ppacs_v2026.04.geo.root";

    // Input event file
    TString fEventFile;
    if (fGenerator.CompareTo("inclroot") == 0)
        fEventFile = "n_U238_500.root";

    // ---- End of Configuration area   ---------------------------------------

    // ---- Stable part   -----------------------------------------------------
    TString dir = gSystem->Getenv("VMCWORKDIR");
    TString pardir = gSystem->Getenv("PARAMDIR");
    if (pardir == "")
    {
        std::cout << "PARAMDIR is not set, this defines the path for input ROOT files, please look at the README."
                  << std::endl;
        gApplication->Terminate();
    }
    std::cout << "Parameters at: " << pardir << std::endl;

    TString geomdir = dir + "/geometry/";
    gSystem->Setenv("GEOMPATH", geomdir.Data());
    geomdir.ReplaceAll("//", "/");

    TString r3b_confdir = dir + "/gconfig/";
    gSystem->Setenv("CONFIG_DIR", r3b_confdir.Data());
    r3b_confdir.ReplaceAll("//", "/");

    // -----   Timer   --------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    // -----   Create simulation run   ----------------------------------------
    auto run = std::make_unique<FairRunSim>();
    run->SetName(fMC); // Transport engine
    auto config = std::make_unique<FairGenericVMCConfig>();

    // run->SetSimulationConfig(new FairVMCConfig());

    run->SetSimulationConfig(std::move(config));
    // run->SetIsMT(isMT);   // Multi-threading mode (Geant4 only)
    run->SetSink(std::make_unique<FairRootFileSink>(OutFile));

    // -----   Runtime data base   --------------------------------------------
    FairRuntimeDb* rtdb = run->GetRuntimeDb();

    // -----   Load detector parameters    ------------------------------------

    // ----- Containers
    UInt_t runId = 1;
    rtdb->initContainers(runId);

    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media_r3b.geo"); // Materials

    // -----   Create R3B geometry --------------------------------------------

    // Cave definition
    auto* cave = new R3BCave("CAVE");
    cave->SetGeometryFileName("r3b_cave_vacuum.geo");
    run->AddModule(cave);

    auto* ppacs = new NTOFPpacs(fPpacsGeo);
    run->AddModule(ppacs);

    // -----   Create PrimaryGenerator   --------------------------------------

    // 1 - Create the Main API class for the Generator
    auto* primGen = new FairPrimaryGenerator();

    if (fGenerator.CompareTo("box") == 0)
    {
        // Define the BOX generator
        int pdgId = 2112;   // neutron beam
        double theta1 = 0.; // polar angle distribution
        double theta2 = 0.;
        double momentum = 1.50;
        auto* boxGen = new FairBoxGenerator(pdgId, 1);
        boxGen->SetThetaRange(theta1, theta2);
        boxGen->SetPRange(momentum, momentum);
        boxGen->SetPhiRange(0., 360.);
        boxGen->SetXYZ(0.0, 0.0, 0.);
        primGen->AddGenerator(boxGen);
    }

    if (fGenerator.CompareTo("inclroot") == 0)
    {
        auto* gen = new R3BINCLRootGenerator((pardir + "/input/" + fEventFile).Data());
        gen->SetOnlyFission();
        gen->SetXYZ(0., 0., -138.);
        gen->SetDxDyDz(0.5, 0.5, 0.5);
        primGen->AddGenerator(gen);
    }

    run->SetGenerator(primGen);

    //-------Set visualisation flag to true------------------------------------
    run->SetStoreTraj(fVis);

    // -----   Initialize simulation run   ------------------------------------
    run->Init();

    // -----   Runtime database   ---------------------------------------------
    bool kParameterMerged = kTRUE;
    auto parOut = new FairParRootFileIo(kParameterMerged);
    parOut->open(ParFile.Data());
    rtdb->setOutput(parOut);
    rtdb->saveOutput();
    rtdb->print();

    // -----   Start run   ----------------------------------------------------
    if (nEvents > 0)
        run->Run(nEvents);

    // -----   Finish   -------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime() / 60.;
    Double_t ctime = timer.CpuTime() / 60.;
    cout << endl << endl;
    cout << "Macro finished successfully." << endl;
    cout << "Output file is " << OutFile << endl;
    cout << "Parameter file is " << ParFile << endl;
    cout << "Real time " << rtime << " min, CPU time " << ctime << " min" << endl << endl;

    cout << " Test passed" << endl;
    cout << " All ok " << endl;
    gApplication->Terminate();
}
