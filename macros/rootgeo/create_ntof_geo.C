#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TMath.h"
#include "TSystem.h"
#include "TFile.h"
#include "TROOT.h"
#include <iostream>

TGeoVolume* CreateTarget(const char* name,
                         TGeoMedium* pMedTarget,
                         TGeoMedium* pMedWorld,
                         double target_x,
                         double target_y,
                         double target_thickness,
                         bool withBacking = false,
                         TGeoMedium* pMedBacking = nullptr,
                         double backing_thickness = 0.0,
                         int activeColor = kMagenta + 1,
                         int backingColor = kGray + 2)
{
    double total_z = target_thickness + (withBacking ? backing_thickness : 0.0);

    auto* targetShape = new TGeoBBox(Form("%sShape", name),
                                     target_x / 2.0,
                                     target_y / 2.0,
                                     total_z / 2.0);

    auto* targetVol = new TGeoVolume(name, targetShape, pMedWorld);
    targetVol->SetLineColor(kGray + 1);
    targetVol->SetFillColor(kGray + 1);
    targetVol->SetTransparency(70);
    targetVol->SetVisLeaves(kTRUE);

    auto* activeShape = new TGeoBBox(Form("%sActiveShape", name),
                                     target_x / 2.0,
                                     target_y / 2.0,
                                     target_thickness / 2.0);

    auto* activeVol = new TGeoVolume(Form("%s_active", name), activeShape, pMedTarget);
    activeVol->SetLineColor(activeColor);
    activeVol->SetFillColor(activeColor);
    activeVol->SetVisLeaves(kTRUE);

    if (!withBacking) {
        targetVol->AddNode(activeVol, 1, new TGeoTranslation(0., 0., 0.));
        return targetVol;
    }

    if (!pMedBacking)
        Fatal("CreateTarget", "Backing requested but pMedBacking is null");

    auto* backingShape = new TGeoBBox(Form("%sBackingShape", name),
                                      target_x / 2.0,
                                      target_y / 2.0,
                                      backing_thickness / 2.0);

    auto* backingVol = new TGeoVolume(Form("%s_backing", name), backingShape, pMedBacking);
    backingVol->SetLineColor(backingColor);
    backingVol->SetFillColor(backingColor);
    backingVol->SetVisLeaves(kTRUE);

    double z_active  = -backing_thickness / 2.0;
    double z_backing = +target_thickness / 2.0;

    targetVol->AddNode(activeVol,  1, new TGeoTranslation(0., 0., z_active));
    targetVol->AddNode(backingVol, 1, new TGeoTranslation(0., 0., z_backing));

    return targetVol;
}
void AddTarget(TGeoVolume* mother,
                       TGeoVolume* target,
                       int copyNo,
                       double x_ppac,
                       double y_ppac,
                       double z_ppac,
                       double distance,
                       double rotYdeg = 45.0)
{
    double theta = TMath::DegToRad() * rotYdeg;

    double dx = distance * TMath::Sin(theta);
    double dz = distance * TMath::Cos(theta);

    double x_target = x_ppac + dx;
    double y_target = y_ppac;
    double z_target = z_ppac + dz;

    auto* rot = new TGeoRotation();
    rot->RotateY(rotYdeg);

    auto* tr = new TGeoCombiTrans(x_target, y_target, z_target, rot);
    mother->AddNode(target, copyNo, tr);
}
TGeoVolume* CreatePPAC(const char* name,
                       TGeoMedium* pMedGas,
                       TGeoMedium* pMedMylar,
                       double ppac_x,
                       double ppac_y,
                       double mylar_thickness,
                       double gas_gap)
{
    double ppac_z = 2.0 * gas_gap + 3.0 * mylar_thickness;

    // --- Volumen contenedor (sin cambio) ---
    auto* ppacShape = new TGeoBBox(Form("%sShape", name),
                                   ppac_x / 2.0,
                                   ppac_y / 2.0,
                                   ppac_z / 2.0);
    auto* ppacVol = new TGeoVolume(name, ppacShape, pMedGas);
    ppacVol->SetLineColor(kCyan);
    ppacVol->SetFillColor(kCyan);
    ppacVol->SetVisLeaves(kTRUE);

    // --- Láminas de mylar (sin cambio) ---
    auto* mylarShape = new TGeoBBox(Form("%sMylarShape", name),
                                    ppac_x / 2.0,
                                    ppac_y / 2.0,
                                    mylar_thickness / 2.0);

    auto* cathode_y = new TGeoVolume(Form("%s_cathode_y", name), mylarShape, pMedMylar);
    auto* anode     = new TGeoVolume(Form("%s_anode",     name), mylarShape, pMedMylar);
    auto* cathode_x = new TGeoVolume(Form("%s_cathode_x", name), mylarShape, pMedMylar);

    cathode_y->SetLineColor(kBlue);   cathode_y->SetFillColor(kBlue);
    anode->SetLineColor(kRed);        anode->SetFillColor(kRed);
    cathode_x->SetLineColor(kGreen+2); cathode_x->SetFillColor(kGreen+2);
    cathode_y->SetVisLeaves(kTRUE);
    anode->SetVisLeaves(kTRUE);
    cathode_x->SetVisLeaves(kTRUE);

    // --- Volúmenes de gas activo (NUEVO) ---
    auto* gasShape = new TGeoBBox(Form("%sGasShape", name),
                                  ppac_x / 2.0,
                                  ppac_y / 2.0,
                                  gas_gap / 2.0);

    // El nombre debe contener "Ppac_gas_" para que CheckIfSensitive los detecte
    auto* gas1 = new TGeoVolume(Form("Ppac_gas_1_%s", name), gasShape, pMedGas);
    auto* gas2 = new TGeoVolume(Form("Ppac_gas_2_%s", name), gasShape, pMedGas);

    gas1->SetLineColor(kCyan - 7); gas1->SetFillColor(kCyan - 7);
    gas2->SetLineColor(kCyan - 7); gas2->SetFillColor(kCyan - 7);
    gas1->SetVisLeaves(kTRUE);
    gas2->SetVisLeaves(kTRUE);

    double z_cathode_y = -(gas_gap + mylar_thickness);
    double z_gas1      = -(gas_gap + mylar_thickness) / 2.0;
    double z_anode     =   0.0;
    double z_gas2      = +(gas_gap + mylar_thickness) / 2.0;
    double z_cathode_x = +(gas_gap + mylar_thickness);

    ppacVol->AddNode(cathode_y, 1, new TGeoTranslation(0., 0., z_cathode_y));
    ppacVol->AddNode(gas1,      1, new TGeoTranslation(0., 0., z_gas1));
    ppacVol->AddNode(anode,     1, new TGeoTranslation(0., 0., z_anode));
    ppacVol->AddNode(gas2,      1, new TGeoTranslation(0., 0., z_gas2));
    ppacVol->AddNode(cathode_x, 1, new TGeoTranslation(0., 0., z_cathode_x));

    return ppacVol;
}

void AddPPAC(TGeoVolume* mother,
             TGeoVolume* ppac,
             int copyNo,
             double x,
             double y,
             double z,
             double rotYdeg = 45.0)
{
    auto* rot = new TGeoRotation();
    rot->RotateY(rotYdeg);

    auto* tr = new TGeoCombiTrans(x, y, z, rot);
    mother->AddNode(ppac, copyNo, tr);
}

void create_ntof_geo(const char* geoTag = "v2026.04")
{
    TGeoManager* gGeoMan = nullptr;

    double worldX = 500.;
    double worldY = 500.;
    double worldZ = 1630.;

    const int n_PPAC = 10;
    const double d_PPACs = 50.;
    const double d_target_PPAC = 25.;

    const double target_ce_thickness = 3.7797657e-3;
    const double target_u_thickness = 0.41105294e-3;
    const double target_gold_thickness = 0.15535187e-3;
    const double backing_thickness = 2.5e-3;
    const double radius_target = 40;

    double ppac_x = 200;
    double ppac_y = 200;
    double mylar_thickness = 1.7e-3;
    double gas_gap = 3.2;

    FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();

    TString geoPath = gSystem->Getenv("VMCWORKDIR");
    std::cout << "Loading geomedia" << std::endl;
    TString medFile = "/nucl_lustre/nicolas_sanchez/R3BRoot/R3BRoot/geometry/media_r3b.geo";
    geoFace->setMediaFile(medFile);
    geoFace->readMedia();

    gGeoMan = gGeoManager;

    TString geoFileName = geoPath + "/geometry/target_area_ppacs_";
    geoFileName += geoTag;
    geoFileName += ".geo.root";

    FairGeoMedia* geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

    std::cout << "Beginning materials" << std::endl;

    // octofluoropropane
    FairGeoMedium* mOcto = geoMedia->getMedium("octofluoropropane");
    if (!mOcto) Fatal("Main", "FairMedium octofluoropropane not found");
    geoBuild->createMedium(mOcto);
    TGeoMedium* pMedOcto = gGeoMan->GetMedium("octofluoropropane");
    if (!pMedOcto) Fatal("Main", "Medium octofluoropropane not found");

    // mylar
    FairGeoMedium* mMylar = geoMedia->getMedium("mylar");
    if (!mMylar) Fatal("Main", "FairMedium mylar not found");
    geoBuild->createMedium(mMylar);
    TGeoMedium* pMedMylar = gGeoMan->GetMedium("mylar");
    if (!pMedMylar) Fatal("Main", "Medium mylar not found");

    // aluminium
    FairGeoMedium* mAl = geoMedia->getMedium("aluminium");
    if (!mAl) Fatal("Main", "FairMedium aluminium not found");
    geoBuild->createMedium(mAl);
    TGeoMedium* pMedAl = gGeoMan->GetMedium("aluminium");
    if (!pMedAl) Fatal("Main", "Medium aluminium not found");

    // cerium oxide
    FairGeoMedium* mCeO = geoMedia->getMedium("cerium_oxide");
    if (!mCeO) Fatal("Main", "FairMedium cerium_oxide not found");
    geoBuild->createMedium(mCeO);
    TGeoMedium* pMedCeO = gGeoMan->GetMedium("cerium_oxide");
    if (!pMedCeO) Fatal("Main", "Medium cerium_oxide not found");

    // uranium oxide
    FairGeoMedium* mUO = geoMedia->getMedium("uranium_oxide");
    if (!mUO) Fatal("Main", "FairMedium uranium_oxide not found");
    geoBuild->createMedium(mUO);
    TGeoMedium* pMedUO = gGeoMan->GetMedium("uranium_oxide");
    if (!pMedUO) Fatal("Main", "Medium uranium_oxide not found");

    // gold
    FairGeoMedium* mGold = geoMedia->getMedium("gold");
    if (!mGold) Fatal("Main", "FairMedium gold not found");
    geoBuild->createMedium(mGold);
    TGeoMedium* pMedGold = gGeoMan->GetMedium("gold");
    if (!pMedGold) Fatal("Main", "Medium gold not found");

    std::cout << "Materials implemented" << std::endl;

    gGeoMan = static_cast<TGeoManager*>(gROOT->FindObject("FAIRGeom"));
    if (!gGeoMan) return;

    gGeoMan->SetName("NTOFgeom");
    auto* top = new TGeoVolumeAssembly("TOP");
    gGeoMan->SetTopVolume(top);

    auto* worldShape = new TGeoBBox("WorldBox", worldX / 2., worldY / 2., worldZ / 2.);
    auto* worldVol = new TGeoVolume("World", worldShape, pMedOcto);
    worldVol->SetLineColor(kBlack);
    top->AddNode(worldVol, 1);

    auto* ppacVol = CreatePPAC("PPAC",
                               pMedOcto,
                               pMedMylar,
                               ppac_x,
                               ppac_y,
                               mylar_thickness,
                               gas_gap);
    

    double z0 = -0.5 * (n_PPAC - 1) * d_PPACs; //center PPACs around 0
    auto* targetCe = CreateTarget("Target_Ce",
                              pMedCeO,
                              pMedOcto,
                              radius_target,
                              radius_target,
                              target_ce_thickness,
                              true,
                              pMedAl,
                              backing_thickness,
                              kOrange + 1,
                              kGray + 2);

    auto* targetU = CreateTarget("Target_U",
                             pMedUO,
                             pMedOcto,
                             radius_target,
                             radius_target,
                             target_u_thickness,
                             true,
                             pMedAl,
                             backing_thickness,
                             kMagenta + 1,
                             kGray + 2);

    auto* targetAu = CreateTarget("Target_Au",
                              pMedGold,
                              pMedOcto,
                              radius_target,
                              radius_target,
                              target_gold_thickness,
                              false,
                              nullptr,
                              0.0,
                              kYellow,
                              kGray + 2);

    for (int i = 0; i < n_PPAC; ++i) {

        double x_ppac = 0.0;
        double y_ppac = 0.0;
        double z_ppac = z0 + i * d_PPACs;

        // PPACs
        AddPPAC(worldVol, ppacVol, i + 1, x_ppac, y_ppac, z_ppac, 45.0);

        // targets
        if (i >= 0 && i <= 6) {
            AddTarget(worldVol, targetCe, i + 1,
                         x_ppac, y_ppac, z_ppac,
                         d_target_PPAC, 45.0);
        }
        else if (i == 7) {
            AddTarget(worldVol, targetAu, i + 1,
                         x_ppac, y_ppac, z_ppac,
                         d_target_PPAC, 45.0);
        }
        else if (i == 8) {
            AddTarget(worldVol, targetU, i + 1,
                         x_ppac, y_ppac, z_ppac,
                         d_target_PPAC, 45.0);
    }
}

    std::cout << "Detector implemented" << std::endl;

    std::cout << "Closing geometry" << std::endl;
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
}