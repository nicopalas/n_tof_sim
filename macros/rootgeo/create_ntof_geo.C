#include "TGeoManager.h"
#include "TMath.h"
#include "TSystem.h"
#include "TFile.h"
#include "TROOT.h"
#include <iostream>

void create_ntof_geo(const char* geoTag = "v2026.04")
{
    TGeoManager* gGeoMan = nullptr;
    double worldX = 50.;
    double worldY = 50.;
    double worldZ = 600.;

    double ppac_x = 20.0;
    double ppac_y= 20.0;
    double mylar_thickness = 1.7e-4;
    double gas_gap = 0.32;
    double ppac_z = 2*gas_gap+3*mylar_thickness;

// -------   Load media from media file   -----------------------------------
    FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();


    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    std::cout << "Loading geomedia" << std::endl;
    TString medFile = "/nucl_lustre/nicolas_sanchez/R3BRoot/R3BRoot/geometry/media_r3b.geo";
    geoFace->setMediaFile(medFile);
    geoFace->readMedia();

    gGeoMan = gGeoManager;
    // --------------------------------------------------------------------------

    // -------   Geometry file name (output)   ----------------------------------
    TString geoFileName = geoPath + "/geometry/target_area_ppacs_";
    geoFileName += geoTag;
    geoFileName += ".geo.root";

    // -----------------   Get and create the required media    -----------------
    FairGeoMedia* geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();
    std::cout << "Beginning materials" << std::endl;
    // octofluoropropane
    FairGeoMedium* mOcto = geoMedia->getMedium("octofluoropropane");
    if (!mOcto)
        Fatal("Main", "FairMedium octofluoropropane not found");
    geoBuild->createMedium(mOcto);
    TGeoMedium* pMedOcto = gGeoMan->GetMedium("octofluoropropane");
    if (!pMedOcto)
        Fatal("Main", "Medium octofluoropropane not found");
    std::cout << "Octofluropropane" << std::endl;
    // kapton
    FairGeoMedium* mKapton = geoMedia->getMedium("kapton");
    if (!mKapton)
        Fatal("Main", "FairMedium kapton not found");
    geoBuild->createMedium(mKapton);
    TGeoMedium* pMedKapton = gGeoMan->GetMedium("kapton");
    if (!pMedKapton)
        Fatal("Main", "Medium kapton not found");
    
    // mylar
    FairGeoMedium* mMylar = geoMedia->getMedium("mylar");
    if (!mMylar)
        Fatal("Main", "FairMedium mylar not found");
    geoBuild->createMedium(mMylar);
    TGeoMedium* pMedMylar = gGeoMan->GetMedium("mylar");
    if (!pMedMylar)
        Fatal("Main", "Medium mylar not found");
    
    // gold
    FairGeoMedium* mGold = geoMedia->getMedium("gold");
    if (!mGold)
        Fatal("Main", "FairMedium gold not found");
    geoBuild->createMedium(mGold);
    TGeoMedium* pMedGold = gGeoMan->GetMedium("gold");
    if (!pMedGold)
        Fatal("Main", "Medium gold not found");
    
    // aluminium
    FairGeoMedium* mAl = geoMedia->getMedium("aluminium");
    if (!mAl)
        Fatal("Main", "FairMedium aluminium not found");
    geoBuild->createMedium(mAl);
    TGeoMedium* pMedAl = gGeoMan->GetMedium("aluminium");
    if (!pMedAl)
        Fatal("Main", "Medium aluminium not found");
    
    // cerium oxide
    FairGeoMedium* mCeO = geoMedia->getMedium("cerium_oxide");
    if (!mCeO)
        Fatal("Main", "FairMedium cerium_oxide not found");
    geoBuild->createMedium(mCeO);
    TGeoMedium* pMedCeO = gGeoMan->GetMedium("cerium_oxide");
    if (!pMedCeO)
        Fatal("Main", "Medium cerium_oxide not found");
    
    // uranium oxide
    FairGeoMedium* mUO = geoMedia->getMedium("uranium_oxide");
    if (!mUO)
        Fatal("Main", "FairMedium uranium_oxide not found");
    geoBuild->createMedium(mUO);
    TGeoMedium* pMedUO = gGeoMan->GetMedium("uranium_oxide");
    if (!pMedUO)
        Fatal("Main", "Medium uranium_oxide not found");
    
    // copper
    FairGeoMedium* mCu = geoMedia->getMedium("copper");
    if (!mCu)
        Fatal("Main", "FairMedium copper not found");
    geoBuild->createMedium(mCu);
    TGeoMedium* pMedCu = gGeoMan->GetMedium("copper");
    if (!pMedCu)
    Fatal("Main", "Medium copper not found");
    gGeoMan = new TGeoManager("PPACgeom", "PPAC geometry");
    std::cout << "Materials implemented" << std::endl;
    


    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = static_cast<TGeoManager*>(gROOT->FindObject("FAIRGeom"));

    if (!gGeoMan)
	return;

    gGeoMan->SetName("NTOFgeom");
    auto* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);
    
    // defining top volume
    
    auto* worldShape = new TGeoBBox("WorldBox", worldX/2., worldY/2., worldZ/2.);
    auto* worldVol = new TGeoVolume("World", worldShape, pMedOcto); // should I fill it with gas? only parametrize fission chamber
    worldVol->SetLineColor(kBlack);
    top->AddNode(worldVol, 1);

    auto* ppacShape = new TGeoBBox("PPACBox", ppac_x/2.0, ppac_y/2.0, ppac_z/2.0);
    auto* ppacVol = new TGeoVolume("PPAC", ppacShape, pMedOcto);
    ppacVol->SetLineColor(kCyan);

    worldVol->AddNode(ppacVol,1,new TGeoTranslation(0.,0.,0.));

    auto* mylarShape = new TGeoBBox("mylarBox", ppac_x/2.0,ppac_y/2., mylar_thickness/2.0);
    auto* cathode_y = new TGeoVolume("cathode_y", mylarShape, pMedMylar);
    auto* anode = new TGeoVolume("anode", mylarShape, pMedMylar);
    auto* cathode_x = new TGeoVolume("cathode_x", mylarShape, pMedMylar);
    std::cout << "Detector implemented" << std::endl;
    cathode_y->SetLineColor(kBlue);
    anode->SetLineColor(kRed);
    cathode_x->SetLineColor(kBlue);

    cathode_y->SetVisLeaves(kTRUE);
    cathode_x->SetVisLeaves(kTRUE);
    anode->SetVisLeaves(kTRUE);

    double z_cathode_y = -gas_gap;
    double z_anode = 0.;
    double z_cathode_x = gas_gap;

    ppacVol->AddNode(cathode_y, 1, new TGeoTranslation(0.,0.,z_cathode_y));
    ppacVol->AddNode(cathode_x, 1, new TGeoTranslation(0.,0.,z_cathode_x));
    ppacVol->AddNode(anode, 1, new TGeoTranslation(0.,0.,z_anode));
    



    std::cout << "Closing geometry" <<std::endl;
    gGeoMan->CloseGeometry();
    gGeoMan->CheckOverlaps(1e-5);
    gGeoMan->PrintOverlaps();
    gGeoMan->Test();

    TFile geoFile("ola.root", "RECREATE");
    top->Write();
    geoFile.Close();

    std::cout << "\033[34mCreating geometry:\033[0m "
              << "\033[33m" << geoFileName << "\033[0m" << std::endl;
    std::cout << "Macro finished successfully." << std::endl;
    //gApplication->Terminate();
}
