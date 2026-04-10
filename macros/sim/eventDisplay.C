void eventDisplay()
{
    auto* fRun = new FairRunAna();
    fRun->SetSource(new FairFileSource("sim.root"));
    fRun->SetSink(new FairRootFileSink("test.root"));

    auto* rtdb = fRun->GetRuntimeDb();
    auto* parIo1 = new FairParRootFileIo();
    parIo1->open("par.root");
    rtdb->setFirstInput(parIo1);
    rtdb->print();

    auto* fMan = new R3BEventManager();
    auto* Track = new R3BMCTracks("Monte-Carlo Tracks");

    fMan->AddTask(Track);
    fMan->Init();
    gEve->GetDefaultGLViewer()->SetClearColor(kOrange - 4);
}
