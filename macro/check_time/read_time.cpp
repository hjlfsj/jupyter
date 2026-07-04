#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <sstream>

struct EventData {
    Double_t time_x0;
    Double_t time_x1;
    Double_t time_x2;
    Double_t time_v;
    Int_t num_x;
    Int_t num_v;
    
    EventData() : time_x0(0), time_x1(0), time_x2(0), time_v(0), num_x(0), num_v(0) {}
    
    EventData(Double_t t0, Double_t t1, Double_t t2, Double_t tv, Int_t nx, Int_t nv)
        : time_x0(t0), time_x1(t1), time_x2(t2), time_v(tv), num_x(nx), num_v(nv) {}
};

void print_help(const char* program_name) {
    std::cout << "Usage: " << program_name << " num_x num_v\n\n"
              << "Description:\n"
              << "  Reads time information from four ROOT files and creates/updates an output file.\n\n"
              << "Arguments:\n"
              << "  num_x    - Run number for the first three files\n"
              << "  num_v    - Run number for the fourth file\n\n"
              << "Input files (generated from arguments):\n"
              << "  file_num_x_0.root  - Contains branch \"time\" for num_x run (e.g., file_12345_0.root)\n"
              << "  file_num_x_1.root  - Contains branch \"time\" for num_x run (e.g., file_12345_1.root)\n"
              << "  file_num_x_2.root  - Contains branch \"time\" for num_x run (e.g., file_12345_2.root)\n"
              << "  file_num_v.root    - Contains branch \"time\" for num_v run (e.g., file_67890.root)\n\n"
              << "Output file:\n"
              << "  output.root   - Contains tree \"time_tree\" with branches:\n"
              << "                  time_x0, time_x1, time_x2, time_v, num_x, num_v\n\n"
              << "Note: The program reads the LAST entry from each input file.\n"
              << "      Output entries are sorted by num_x.\n";
}

Double_t read_last_time(const char* filename, const char* branchname = "time") {
    TFile* file = TFile::Open(filename, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        if (file) file->Close();
        exit(1);
    }
    
    TTree* tree = (TTree*)file->Get("tree");  // 假设树名为 "tree"，如需修改请调整
    if (!tree) {
        std::cerr << "Error: Cannot find tree in file " << filename << std::endl;
        file->Close();
        exit(1);
    }
    
    Double_t time_val = 0;
    tree->SetBranchAddress(branchname, &time_val);
    
    Long64_t nentries = tree->GetEntries();
    if (nentries == 0) {
        std::cerr << "Error: Tree in file " << filename << " has no entries" << std::endl;
        file->Close();
        exit(1);
    }
    
    // 读取最后一个 entry
    tree->GetEntry(nentries - 1);
    
    file->Close();
    return time_val;
}


int main(int argc, char* argv[]) {
    // 检查参数
    if (argc < 2) {
        if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
            print_help(argv[0]);
            return 0;
        }
        std::cerr << "Error: Invalid number of arguments\n";
        std::cerr << "Use '" << argv[0] << " --help' for usage information\n";
        return 1;
    }
    
    Int_t num_x = std::atoi(argv[1]);
    Int_t num_v = num_x;
    
    // 使用传入的参数构建文件名
    std::stringstream ss_x0, ss_x1, ss_x2, ss_v;
    ss_x0 << "/data/disk1/ribll2026_www_data/grit/trigger_" << Form("%04d", num_x) << ".root";
    ss_x1 << "/data/disk1/ribll2026_www_data/grit/t0d3_" << Form("%04d", num_x) << ".root";
    ss_x2 << "/data/disk1/ribll2026_www_data/grit/t0s_" << Form("%04d", num_x) << ".root";
    ss_v  << "/data/disk1/ribll2026_www_data/grain/grain_" << Form("%04d", num_v) << ".root";
    
    std::string file_x0_name = ss_x0.str();
    std::string file_x1_name = ss_x1.str();
    std::string file_x2_name = ss_x2.str();
    std::string file_v_name  = ss_v.str();
    
    std::cout << "Input files:" << std::endl;
    std::cout << "  " << file_x0_name << std::endl;
    std::cout << "  " << file_x1_name << std::endl;
    std::cout << "  " << file_x2_name << std::endl;
    std::cout << "  " << file_v_name << std::endl;
    
    // 读取时间值（最后一个 entry）
    std::cout << "\nReading time values..." << std::endl;
    Double_t time_x0 = read_last_time(file_x0_name.c_str(), "time")/1e9;  // 转换为秒
    Double_t time_x1 = read_last_time(file_x1_name.c_str(), "time")/1e9;  // 转换为秒
    Double_t time_x2 = read_last_time(file_x2_name.c_str(), "time")/1e9;  // 转换为秒
    Double_t time_v  = read_last_time(file_v_name.c_str(), "vme_time")/1e9;  // 转换为秒
    
    std::cout << "time_x0 = " << time_x0 << " s" << std::endl;
    std::cout << "time_x1 = " << time_x1 << " s" << std::endl;
    std::cout << "time_x2 = " << time_x2 << " s" << std::endl;
    std::cout << "time_v  = " << time_v << " s" << std::endl;
    
    // 读取或创建输出文件
    const char* output_filename = "check_time.root";
    std::vector<EventData> events;
    
    // 检查输出文件是否存在
    TFile* outfile = TFile::Open(output_filename, "UPDATE");
    if (!outfile || outfile->IsZombie()) {
        if (outfile) outfile->Close();
        outfile = TFile::Open(output_filename, "RECREATE");
        std::cout << "\nCreating new output file: " << output_filename << std::endl;
    } else {
        std::cout << "\nUpdating existing output file: " << output_filename << std::endl;
        
        // 读取现有数据
        TTree* oldtree = (TTree*)outfile->Get("tree");
        if (oldtree) {
            Double_t old_t0, old_t1, old_t2, old_tv;
            Int_t old_nx, old_nv;
            
            oldtree->SetBranchAddress("time_x0", &old_t0);
            oldtree->SetBranchAddress("time_x1", &old_t1);
            oldtree->SetBranchAddress("time_x2", &old_t2);
            oldtree->SetBranchAddress("time_v", &old_tv);
            oldtree->SetBranchAddress("num_x", &old_nx);
            oldtree->SetBranchAddress("num_v", &old_nv);
            
            Long64_t nentries = oldtree->GetEntries();
            for (Long64_t i = 0; i < nentries; i++) {
                oldtree->GetEntry(i);
                if (old_nx == num_x ) continue;
                events.emplace_back(old_t0, old_t1, old_t2, old_tv, old_nx, old_nv);
            }
            
            std::cout << "Read " << nentries << " existing entries" << std::endl;
        }
        outfile->Close();
        outfile = TFile::Open(output_filename, "RECREATE");
    }
    
    // 添加新数据
    events.emplace_back(time_x0, time_x1, time_x2, time_v, num_x, num_v);
    
    // 按 num_x 排序
    std::sort(events.begin(), events.end(), 
              [](const EventData& a, const EventData& b) {
                  return a.num_x < b.num_x;
              });
    
    // 创建新树
    TTree* tree = new TTree("tree", "Time data tree");
    EventData event;
    
    tree->Branch("time_x0", &event.time_x0, "time_x0/D");
    tree->Branch("time_x1", &event.time_x1, "time_x1/D");
    tree->Branch("time_x2", &event.time_x2, "time_x2/D");
    tree->Branch("time_v", &event.time_v, "time_v/D");
    tree->Branch("num_x", &event.num_x, "num_x/I");
    tree->Branch("num_v", &event.num_v, "num_v/I");
    
    // 填充数据
    for (auto& ev : events) {
        event = ev;

        tree->Fill();
    }
    
    // 写入文件
    outfile->cd();
    tree->Write("", TObject::kOverwrite);
    outfile->Close();
    
    std::cout << "\nOutput file created successfully: " << output_filename << std::endl;
    std::cout << "Total entries: " << events.size() << std::endl;
    std::cout << "Added entry: num_x=" << num_x << ", num_v=" << num_v << "  num_v should check run shift" << std::endl;
    
    return 0;
}