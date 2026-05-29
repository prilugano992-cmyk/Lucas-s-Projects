// kernel.c - Genesis OS

#define WHITE_ON_BLACK 0x07

// ==========================================
//    1. DEFINIÇÕES DE ESTRUTURAS (STRUCTS)
// ==========================================

typedef struct {
    int active;
    char name[32];
    int size;
    int is_dir;
    int parent_dir;
    int targets_dir;
    int start_sector;
} GfsFileEntry;

typedef struct {
    unsigned int flags;
    unsigned int mem_lower;
    unsigned int mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count; 
    unsigned int mods_addr;  
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    unsigned int mod_start;  
    unsigned int mod_end;    
    unsigned int cmdline;
    unsigned int pad;
} __attribute__((packed)) multiboot_module_t;

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12]; 
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
} __attribute__((packed)) tar_header_t;

typedef struct {
    int active;
    int x, y, w, h;
    int has_start;
    int has_window_theme;
    int has_power;
    int has_clock;
    const char* name;
} DockGroup;

// ==========================================
//    2. PROTÓTIPOS COMPLETOS DE TODAS AS FUNÇÕES
// ==========================================

static inline void outb(unsigned short port, unsigned char val);
static inline void outw(unsigned short port, unsigned short val);
static inline unsigned char inb(unsigned short port);
static inline unsigned short inw(unsigned short port);
static inline void bga_write(unsigned short index, unsigned short data);
static inline void bga_set_mode(unsigned short width, unsigned short height, unsigned short bpp);
static inline void safe_strcpy(char* dest, const char* src, int max_len);
static inline int strcmp(const char* s1, const char* s2);
static inline int parse_octal(const char* str, int size);
static inline int get_glyph_index(char c);
static inline char* tar_find_file(unsigned char* tar_start, const char* filename, int* out_size);
static inline unsigned char get_update_in_progress_flag(void);
static inline unsigned char get_rtc_register(int reg);
static inline int decode_bcd(unsigned char bcd);
static inline void read_rtc(int* hours, int* minutes, int* seconds);
static inline void format_time_num(int num, char* out);

void ata_read_sector(unsigned int lba, unsigned short* buffer);
void ata_write_sector(unsigned int lba, unsigned short* buffer);
void draw_rect(int x, int y, int w, int h, unsigned int color);
void draw_rounded_rect(int x, int y, int w, int h, int r, unsigned int color);
void draw_char(int x, int y, char c, unsigned int color);
void draw_string(int x, int y, const char* str, unsigned int color);
void draw_aurora_gradient(int w, int h);
void draw_bliss_background(int w, int h);
void draw_cyber_gradient(int w, int h);
void draw_desktop_background(int w, int h, int mode);
void draw_top_taskbar(int w, int mode);
void draw_dock(DockGroup* dock, int mode);
void draw_start_menu(int x, int y, int h, int mode);
void draw_aero_window(int x, int y, int w, int h, const char* title, int mode);
void draw_file_explorer(int x, int y, int w, int h, int mode);
void draw_notepad(int x, int y, int w, int h, int mode);
void draw_paint(int x, int y, int w, int h, int mode);
void trigger_shutdown(unsigned int* fb, int w, int h);
void draw_cursor(int x, int y);
void redraw_screen(unsigned int* fb, int screen_width, int screen_height, int win_open, int st_menu_open, int mouse_x, int mouse_y);
void delay(int count);
void mouse_write(unsigned char data);
void init_mouse(void);

// ==========================================
//    3. VARIÁVEIS GLOBAIS DE ESTADO DO SO
// ==========================================

int screen_width = 1024;
int screen_height = 768;

unsigned int backbuffer[1024 * 768];

// Janela Principal
int window_x = 100;
int window_y = 120;
int window_width = 420;
int window_height = 280;
int window_open = 1;

char window_text_line1[40] = "GENESIS OS ACTIVE";
char window_text_line2[40] = "OPEN FILES ICON TO EXPLORE DIRECTORIES";
char window_text_line3[40] = "DRAG WINDOWS AND DOCKS FREELY!";

// Janela do File Explorer
int file_window_x = 540;
int file_window_y = 120;
int file_window_width = 440;
int file_window_height = 280;
int file_window_open = 1;

// Janela do NOTEPAD
int notepad_open = 0;
int is_dragging_notepad = 0;
int notepad_x = 200;
int notepad_y = 150;
int notepad_width = 440;
int notepad_height = 300;
char notepad_buffer[512] = "";
int notepad_len = 0;

// Janela do PAINT
int paint_open = 0;
int is_dragging_paint = 0;
int paint_x = 350;
int paint_y = 100;
int paint_width = 300;
int paint_height = 300;
unsigned char paint_canvas[512] = {0}; 

// Estados do Teclado para Renomear
int rename_active = 0;
char rename_buffer[32] = "";
int rename_len = 0;

// Estados de arrasto e interação
int start_menu_open = 0;
int start_menu_height = 0; 
int is_dragging_window = 0;
int is_dragging_file_window = 0;
int is_dragging_dock = 0;
int dragged_dock_index = -1;
int last_click_left = 0;

int aero_color_mode = 1; 

// Variáveis globais do relógio CMOS
int frame_counter = 0;
int current_hour = 12;
int current_minute = 0;
int current_second = 0;

GfsFileEntry vfs[6];

int current_dir = 0;         
int selected_file_index = -1; 

// Docks (inicialmente ativos)
DockGroup docks[4] = {
    {1, 40, 700, 120, 48, 1, 0, 0, 0, "SYSTEM"},         
    {1, 310, 700, 260, 48, 0, 1, 0, 0, "APPS"},           
    {1, 690, 700, 110, 48, 0, 0, 1, 0, "POWER"},
    {1, 890, 700, 100, 48, 0, 0, 0, 1, "CLOCK"} 
};

// Novas variáveis para renomeação de dock e barra superior
int rename_dock_active = 0;
int rename_dock_index = -1;
char rename_dock_buffer[32] = "";
int rename_dock_len = 0;
int taskbar_height = 30;   // altura da barra superior

// Tabela de fontes bitmap de 8x8 pixels compactada
unsigned char font_basic[38][8] = {
    {0x18, 0x24, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00}, {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00},
    {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00}, {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00},
    {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00}, {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00},
    {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00}, {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
    {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x4C, 0x38, 0x00},
    {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00}, {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00},
    {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00}, {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00},
    {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00},
    {0x3C, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A, 0x00}, {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00},
    {0x3C, 0x42, 0x30, 0x18, 0x0C, 0x42, 0x3C, 0x00}, {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, {0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00},
    {0x42, 0x42, 0x42, 0x4A, 0x5A, 0x66, 0x42, 0x00}, {0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x00},
    {0x42, 0x42, 0x42, 0x24, 0x18, 0x18, 0x18, 0x00}, {0x7E, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7E, 0x00},
    
    {0x3C, 0x42, 0x46, 0x4A, 0x52, 0x62, 0x3C, 0x00}, {0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00},
    {0x3C, 0x42, 0x02, 0x0C, 0x30, 0x40, 0x7E, 0x00}, {0x3C, 0x42, 0x02, 0x1C, 0x02, 0x42, 0x3C, 0x00},
    {0x08, 0x18, 0x28, 0x48, 0x7E, 0x08, 0x08, 0x00}, {0x7E, 0x40, 0x7C, 0x02, 0x02, 0x42, 0x3C, 0x00},
    {0x3C, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x3C, 0x00}, {0x7E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00},
    {0x3C, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x3C, 0x00}, {0x3C, 0x42, 0x42, 0x3E, 0x02, 0x02, 0x3C, 0x00},

    {0x10, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x10, 0x00}, // >
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}  // !
};

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',    0,
  ' '
};

// ==========================================
//    4. IMPLEMENTAÇÃO DAS FUNÇÕES
// ==========================================

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void bga_write(unsigned short index, unsigned short data) {
    outw(0x01CE, index);
    outw(0x01CF, data);
}

void bga_set_mode(unsigned short width, unsigned short height, unsigned short bpp) {
    bga_write(4, 0);       
    bga_write(1, width);    
    bga_write(2, height);   
    bga_write(3, bpp);      
    bga_write(4, 1 | 0x40); 
}

void safe_strcpy(char* dest, const char* src, int max_len) {
    int i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int parse_octal(const char* str, int size) {
    int val = 0;
    for (int i = 0; i < size; i++) {
        if (str[i] < '0' || str[i] > '7') break;
        val = val * 8 + (str[i] - '0');
    }
    return val;
}

char* tar_find_file(unsigned char* tar_start, const char* filename, int* out_size) {
    unsigned char* ptr = tar_start;
    while (1) {
        tar_header_t* header = (tar_header_t*)ptr;
        if (header->name[0] == '\0') break;
        int size = parse_octal(header->size, 12);
        if (strcmp(header->name, filename) == 0) {
            *out_size = size;
            return (char*)(ptr + 512);
        }
        int blocks = (size + 511) / 512;
        ptr += 512 + (blocks * 512);
    }
    return 0;
}

static inline unsigned char get_update_in_progress_flag() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

static inline unsigned char get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

static inline int decode_bcd(unsigned char bcd) {
    return ((bcd / 16) * 10) + (bcd & 0x0F);
}

static inline void read_rtc(int* hours, int* minutes, int* seconds) {
    while (get_update_in_progress_flag());
    *seconds = decode_bcd(get_rtc_register(0x00));
    *minutes = decode_bcd(get_rtc_register(0x02));
    *hours = decode_bcd(get_rtc_register(0x04));
}

static inline void format_time_num(int num, char* out) {
    out[0] = '0' + (num / 10);
    out[1] = '0' + (num % 10);
    out[2] = '\0';
}

void ata_read_sector(unsigned int lba, unsigned short* buffer) {
    while ((inb(0x1F7) & 0x80) != 0);
    outb(0x1F6, (0xE0 | ((lba >> 24) & 0x0F)));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);
    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);
    while ((inb(0x1F7) & 0x80) != 0);
    while ((inb(0x1F7) & 0x08) == 0);
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0);
    }
}

void ata_write_sector(unsigned int lba, unsigned short* buffer) {
    while ((inb(0x1F7) & 0x80) != 0);
    outb(0x1F6, (0xE0 | ((lba >> 24) & 0x0F)));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x30);
    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);
    while ((inb(0x1F7) & 0x80) != 0);
    while ((inb(0x1F7) & 0x08) == 0);
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]);
    }
}

void draw_rect(int x, int y, int w, int h, unsigned int color) {
    for (int curr_y = y; curr_y < y + h; curr_y++) {
        for (int curr_x = x; curr_x < x + w; curr_x++) {
            if (curr_x >= 0 && curr_x < 1024 && curr_y >= 0 && curr_y < 768) {
                backbuffer[curr_y * 1024 + curr_x] = color;
            }
        }
    }
}

void draw_rounded_rect(int x, int y, int w, int h, int r, unsigned int color) {
    for (int cy = y; cy < y + h; cy++) {
        for (int cx = x; cx < x + w; cx++) {
            int top_left = (cx < x + r) && (cy < y + r);
            int top_right = (cx >= x + w - r) && (cy < y + r);
            int bottom_left = (cx < x + r) && (cy >= y + h - r);
            int bottom_right = (cx >= x + w - r) && (cy >= y + h - r);
            int draw = 1;
            if (top_left) {
                int dx = cx - (x + r);
                int dy = cy - (y + r);
                if (dx*dx + dy*dy > r*r) draw = 0;
            } else if (top_right) {
                int dx = cx - (x + w - r);
                int dy = cy - (y + r);
                if (dx*dx + dy*dy > r*r) draw = 0;
            } else if (bottom_left) {
                int dx = cx - (x + r);
                int dy = cy - (y + h - r);
                if (dx*dx + dy*dy > r*r) draw = 0;
            } else if (bottom_right) {
                int dx = cx - (x + w - r);
                int dy = cy - (y + h - r);
                if (dx*dx + dy*dy > r*r) draw = 0;
            }
            if (draw) {
                if (cx >= 0 && cx < 1024 && cy >= 0 && cy < 768) {
                    backbuffer[cy * 1024 + cx] = color;
                }
            }
        }
    }
}

static inline int get_glyph_index(char c) {
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    if (c == '>' || c == ':') return 36;   // fallback para >
    if (c == '.') return 26;               // fallback para 0 (ponto)
    return 37;                             // !
}

void draw_char(int x, int y, char c, unsigned int color) {
    if (c == ' ' || c == '\n' || c == '\r') return;
    int index = get_glyph_index(c);
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = font_basic[index][row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < 1024 && py >= 0 && py < 768) {
                    backbuffer[py * 1024 + px] = color;
                }
            }
        }
    }
}

void draw_string(int x, int y, const char* str, unsigned int color) {
    int curr_x = x;
    int curr_y = y;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            curr_x = x;
            curr_y += 12;
        } else {
            draw_char(curr_x, curr_y, str[i], color);
            curr_x += 8;
        }
        i++;
    }
}

void draw_aurora_gradient(int w, int h) {
    int r1 = 0, g1 = 229, b1 = 255;
    int r2 = 157, g2 = 255, b2 = 0;
    for (int y = 0; y < h; y++) {
        int r = r1 + ((r2 - r1) * y) / h;
        int g = g1 + ((g2 - g1) * y) / h;
        int b = b1 + ((b2 - b1) * y) / h;
        unsigned int color = (r << 16) | (g << 8) | b;
        for (int x = 0; x < w; x++) {
            backbuffer[y * w + x] = color;
        }
    }
}

void draw_bliss_background(int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int hill_y = 480 + ((x - 512) * (x - 512)) / 1400;
            if (y > hill_y) {
                int g = 140 + (y - hill_y) / 4;
                if (g > 220) g = 220;
                backbuffer[y * w + x] = (40 << 16) | (g << 8) | 50;
            } else {
                int b = 180 + y / 3;
                if (b > 255) b = 255;
                backbuffer[y * w + x] = (100 << 16) | (180 << 8) | b;
            }
        }
    }
}

void draw_cyber_gradient(int w, int h) {
    int r1 = 30, g1 = 0, b1 = 50;
    int r2 = 130, g2 = 0, b2 = 150;
    for (int y = 0; y < h; y++) {
        int r = r1 + ((r2 - r1) * y) / h;
        int g = g1 + ((g2 - g1) * y) / h;
        int b = b1 + ((b2 - b1) * y) / h;
        unsigned int color = (r << 16) | (g << 8) | b;
        for (int x = 0; x < w; x++) {
            backbuffer[y * w + x] = color;
        }
    }
}

void draw_desktop_background(int w, int h, int mode) {
    if (mode == 0) draw_aurora_gradient(w, h);
    else if (mode == 1) draw_bliss_background(w, h);
    else draw_cyber_gradient(w, h);
}

// Barra de tarefas superior
void draw_top_taskbar(int w, int mode) {
    unsigned int bg = 0x0E5E80;
    if (mode == 1) bg = 0x2471A3;
    else if (mode == 2) bg = 0x11021F;
    draw_rect(0, 0, w, taskbar_height, bg);
    draw_rect(0, taskbar_height - 2, w, 2, 0x80DFFF);

    draw_string(8, 8, "Genesis OS", 0xFFFFFF);

    int btn_x = 140;
    if (notepad_open) {
        draw_rect(btn_x, 4, 80, 22, 0x27AE60);
        draw_string(btn_x + 10, 9, "NOTEPAD", 0xFFFFFF);
        btn_x += 90;
    }
    if (paint_open) {
        draw_rect(btn_x, 4, 70, 22, 0xE67E22);
        draw_string(btn_x + 10, 9, "PAINT", 0xFFFFFF);
        btn_x += 80;
    }

    char time_str[10];
    format_time_num(current_hour, time_str);
    time_str[2] = ':';
    format_time_num(current_minute, time_str + 3);
    time_str[5] = ':';
    format_time_num(current_second, time_str + 6);
    time_str[8] = '\0';
    draw_string(w - 90, 9, time_str, 0xFFFFFF);
}

void draw_dock(DockGroup* dock, int mode) {
    if (!dock->active) return;

    unsigned int body_color = 0x0E5E80;
    unsigned int border_color = 0x80DFFF;
    unsigned int handle_color = 0x0A3B52;

    if (mode == 1) {
        body_color = 0x2471A3;
        border_color = 0x5DADE2;
        handle_color = 0x1B4F72;
    } else if (mode == 2) {
        body_color = 0x1A052E;
        border_color = 0xD017FF;
        handle_color = 0x300554;
    }

    draw_rounded_rect(dock->x, dock->y, dock->w, dock->h, 8, body_color);
    draw_rect(dock->x, dock->y, dock->w, 12, handle_color);
    draw_rect(dock->x, dock->y, dock->w, 2, border_color);
    draw_string(dock->x + 6, dock->y + 2, dock->name, 0xFFFFFF);

    // Botão X no canto direito do título
    draw_rect(dock->x + dock->w - 16, dock->y + 2, 12, 12, 0xC0392B);
    draw_char(dock->x + dock->w - 14, dock->y + 4, 'X', 0xFFFFFF);

    if (dock->has_start) {
        unsigned int btn_color = (mode == 1) ? 0x27AE60 : ((mode == 2) ? 0xD017FF : 0x2ECC71);
        unsigned int btn_high = (mode == 1) ? 0x58D68D : ((mode == 2) ? 0xF194FF : 0x82E0AA);
        draw_rect(dock->x + 10, dock->y + 16, 100, 24, btn_color);
        draw_rect(dock->x + 10, dock->y + 16, 100, 4, btn_high);
        draw_string(dock->x + 36, dock->y + 24, "START", 0xFFFFFF);
    }
    else if (dock->has_window_theme) {
        draw_rect(dock->x + 10, dock->y + 16, 75, 24, 0x1F2A38);
        draw_rect(dock->x + 10, dock->y + 16, 75, 2, 0x52BE80);
        draw_string(dock->x + 18, dock->y + 24, "WINDOW", 0xFFFFFF);

        draw_rect(dock->x + 95, dock->y + 16, 75, 24, 0x1F2A38);
        draw_rect(dock->x + 95, dock->y + 16, 75, 2, 0xF39C12);
        draw_string(dock->x + 112, dock->y + 24, "FILES", 0xFFFFFF);

        draw_rect(dock->x + 180, dock->y + 16, 70, 24, 0x1F2A38);
        draw_rect(dock->x + 180, dock->y + 16, 70, 2, 0xAF7AC5);
        draw_string(dock->x + 192, dock->y + 24, "THEME", 0xFFFFFF);
    }
    else if (dock->has_power) {
        draw_rect(dock->x + 10, dock->y + 16, 90, 24, 0xC0392B);
        draw_rect(dock->x + 10, dock->y + 16, 90, 4, 0xE74C3C);
        draw_string(dock->x + 16, dock->y + 24, "SHUTDOWN", 0xFFFFFF);
    }
    else if (dock->has_clock) {
        char time_str[10];
        format_time_num(current_hour, time_str);
        time_str[2] = ':';
        format_time_num(current_minute, time_str + 3);
        time_str[5] = ':';
        format_time_num(current_second, time_str + 6);
        time_str[8] = '\0';
        draw_string(dock->x + 18, dock->y + 24, time_str, 0xFFFFFF);
    }
}

void draw_start_menu(int x, int y, int h, int mode) {
    if (h <= 0) return;
    unsigned int menu_bg = 0x0F3C4F;
    unsigned int border_color = 0x48C9B0;

    if (mode == 1) {
        menu_bg = 0x1B4F72;
        border_color = 0x3498DB;
    } else if (mode == 2) {
        menu_bg = 0x1F003C;
        border_color = 0xD017FF;
    }

    draw_rect(x, y, 160, h, menu_bg);
    draw_rect(x, y, 160, 2, border_color);
    draw_rect(x, y, 2, h, border_color);
    draw_rect(x + 158, y, 2, h, border_color);
    draw_rect(x, y + h - 2, 160, 2, border_color);
    
    if (h >= 40) draw_string(x + 15, y + 20, "SYSTEM INFO", 0xFFFFFF);
    if (h >= 70) draw_string(x + 15, y + 50, "NOTEPAD", 0xFFFFFF);
    if (h >= 100) draw_string(x + 15, y + 80, "PAINT", 0xFFFFFF);
    if (h >= 130) draw_string(x + 15, y + 110, "AERO EFFECTS", border_color);
    if (h >= 160) draw_string(x + 15, y + 140, "SHUTDOWN", 0xEC7063);
}

void draw_aero_window(int x, int y, int w, int h, const char* title, int mode) {
    draw_rounded_rect(x, y, w, h, 12, 0xF2F4F4);
    if (mode == 1) draw_rounded_rect(x, y, w, h, 12, 0xF8F9F9);
    else if (mode == 2) draw_rounded_rect(x, y, w, h, 12, 0x1A052E);

    unsigned int header_color = 0x0E6680;
    unsigned int header_high = 0x3ED7FF;
    if (mode == 1) { header_color = 0x2471A3; header_high = 0x5DADE2; }
    else if (mode == 2) { header_color = 0x4A006E; header_high = 0xBB00FF; }

    draw_rect(x, y, w, 28, header_color);
    draw_rect(x, y, w, 3, header_high);
    draw_rect(x, y + 27, w, 1, 0x073545);
    
    unsigned int text_color = (mode == 2) ? 0x00FF88 : 0xFFFFFF;
    draw_string(x + 8, y + 10, title, text_color);
    
    draw_rect(x + w - 24, y + 4, 20, 20, 0xC0392B);
    draw_rect(x + w - 24, y + 4, 20, 3, 0xE74C3C);
    draw_char(x + w - 18, y + 10, 'X', 0xFFFFFF);
}

void draw_notepad(int x, int y, int w, int h, int mode) {
    draw_aero_window(x, y, w, h, "NOTEPAD", mode);
    draw_rect(x + 10, y + 36, w - 20, h - 80, 0xFFFFFF);
    draw_rect(x + 10, y + 36, w - 20, 1, 0x999999);
    draw_string(x + 15, y + 42, notepad_buffer, 0x000000);

    int btn_y = y + h - 36;
    draw_rect(x + 10, btn_y, 90, 24, 0x27AE60);
    draw_string(x + 40, btn_y + 8, "SAVE", 0xFFFFFF);
}

void draw_paint(int x, int y, int w, int h, int mode) {
    draw_aero_window(x, y, w, h, "GENESIS PAINT", mode);
    int canvas_x = x + 10, canvas_y = y + 36;
    draw_rect(canvas_x, canvas_y, 128, 128, 0xFFFFFF);
    for (int r = 0; r < 64; r++) {
        for (int c = 0; c < 64; c++) {
            int bit_pos = r * 64 + c;
            int set = (paint_canvas[bit_pos / 8] & (1 << (bit_pos % 8)));
            unsigned int color = set ? 0x000000 : 0xFFFFFF;
            draw_rect(canvas_x + c * 2, canvas_y + r * 2, 2, 2, color);
        }
    }
    draw_string(x + 150, y + 50, "USE MOUSE TO\nDRAW IN CANVAS!", 0x000000);
    int btn_y = y + h - 36;
    draw_rect(x + 10, btn_y, 90, 24, 0x27AE60);
    draw_string(x + 40, btn_y + 8, "SAVE", 0xFFFFFF);
}

void draw_file_explorer(int x, int y, int w, int h, int mode) {
    draw_aero_window(x, y, w, h, "FILE EXPLORER", mode);
    draw_rect(x + 10, y + 36, w - 20, 20, 0xFFFFFF);
    draw_rect(x + 10, y + 36, w - 20, 1, 0x999999);
    if (current_dir == 0) draw_string(x + 15, y + 41, "GenesisOS/", 0x000000);
    else draw_string(x + 15, y + 41, "GenesisOS/documents/", 0x000000);

    int list_y = y + 64;
    draw_rect(x + 10, list_y, w - 20, 160, 0xFFFFFF);
    draw_rect(x + 10, list_y, w - 20, 1, 0x999999);

    int start_item_y = list_y + 8;
    int visible_count = 0;
    for (int i = 0; i < 6; i++) {
        if (vfs[i].active && vfs[i].parent_dir == current_dir) {
            int item_y = start_item_y + visible_count * 20;
            if (selected_file_index == i) draw_rect(x + 12, item_y - 2, w - 24, 18, 0x00A2E8);
            if (vfs[i].is_dir) draw_string(x + 15, item_y, "[DIR]", (selected_file_index == i) ? 0xFFFFFF : 0x27AE60);
            else draw_string(x + 15, item_y, "[TXT]", (selected_file_index == i) ? 0xFFFFFF : 0x2980B9);
            draw_string(x + 65, item_y, vfs[i].name, (selected_file_index == i) ? 0xFFFFFF : 0x000000);
            visible_count++;
        }
    }

    int btn_y = y + 240;
    draw_rect(x + 10, btn_y, 75, 24, 0x27AE60); draw_string(x + 30, btn_y + 8, "OPEN", 0xFFFFFF);
    draw_rect(x + 95, btn_y, 75, 24, 0x2980B9); draw_string(x + 115, btn_y + 8, "INFO", 0xFFFFFF);
    draw_rect(x + 180, btn_y, 75, 24, 0xC0392B); draw_string(x + 200, btn_y + 8, "DEL", 0xFFFFFF);
    draw_rect(x + 265, btn_y, 75, 24, 0xF39C12); draw_string(x + 275, btn_y + 8, "RENAME", 0xFFFFFF);
    draw_rect(x + 350, btn_y, 80, 24, 0x7F8C8D); draw_string(x + 356, btn_y + 8, "NEW", 0xFFFFFF);
}

void trigger_shutdown(unsigned int* fb, int w, int h) {
    for (int i = 0; i < w * h; i++) backbuffer[i] = 0x000000;
    int box_x = w / 2 - 240, box_y = h / 2 - 80;
    draw_rect(box_x, box_y, 480, 160, 0xD35400);
    draw_rect(box_x + 4, box_y + 4, 472, 152, 0x000000);
    draw_string(box_x + 45, box_y + 40, "IT IS NOW SAFE TO TURN OFF", 0xD35400);
    draw_string(box_x + 120, box_y + 70, "YOUR COMPUTER.", 0xD35400);
    draw_string(box_x + 90, box_y + 110, "GENESIS OS SHUTDOWN CODE 0", 0x7F8C8D);
    for (int i = 0; i < w * h; i++) fb[i] = backbuffer[i];
    while (1) __asm__ volatile("cli; hlt");
}

void draw_cursor(int x, int y) {
    const char* sprite[12] = {
        "B...........", "BB..........", "BWB.........", "BWWB........",
        "BWWWB.......", "BWWWWB......", "BWWWWWB.....", "BWWWWWWB....",
        "BWWWWWWWB...", "BWWBBBBBB...", "BWB.........", "B..........."
    };
    for (int r = 0; r < 12; r++) {
        for (int c = 0; c < 12; c++) {
            char pixel = sprite[r][c];
            int px = x + c, py = y + r;
            if (px >= 0 && px < 1024 && py >= 0 && py < 768) {
                if (pixel == 'B') backbuffer[py * 1024 + px] = 0x000000;
                else if (pixel == 'W') backbuffer[py * 1024 + px] = 0xFFFFFF;
            }
        }
    }
}

void redraw_screen(unsigned int* fb, int screen_width, int screen_height, int win_open, int st_menu_open, int mouse_x, int mouse_y) {
    // Atualiza relógio periodicamente
    frame_counter++;
    if (frame_counter >= 30) {
        frame_counter = 0;
        read_rtc(&current_hour, &current_minute, &current_second);
    }
    
    if (start_menu_open && start_menu_height < 170) {
        start_menu_height += (170 - start_menu_height) / 3 + 2;
        if (start_menu_height > 170) start_menu_height = 170;
    } else if (!start_menu_open && start_menu_height > 0) {
        start_menu_height -= start_menu_height / 3 + 2;
        if (start_menu_height < 0) start_menu_height = 0;
    }

    for (int i = 0; i < 4; i++) {
        if (docks[i].active && !is_dragging_dock) {
            if (docks[i].y > 660 && docks[i].y != 700) {
                docks[i].y += (700 - docks[i].y) / 4;
                if (docks[i].y > 698) docks[i].y = 700;
            } else if (docks[i].y < 100 && docks[i].y != 20) {
                docks[i].y += (20 - docks[i].y) / 4;
                if (docks[i].y < 22) docks[i].y = 20;
            } else if (docks[i].x < 100 && docks[i].x != 10) {
                docks[i].x += (10 - docks[i].x) / 4;
                if (docks[i].x < 12) docks[i].x = 10;
            }
        }
    }

    draw_desktop_background(screen_width, screen_height, aero_color_mode);
    draw_top_taskbar(screen_width, aero_color_mode);   // barra superior com relógio e indicadores

    for (int i = 0; i < 4; i++) draw_dock(&docks[i], aero_color_mode);

    if (start_menu_height > 0) {
        int menu_x = docks[0].x;
        int menu_y = docks[0].y - start_menu_height;
        if (menu_y < 0) menu_y = 0;
        draw_start_menu(menu_x, menu_y, start_menu_height, aero_color_mode);
    }

    if (win_open) {
        draw_aero_window(window_x, window_y, window_width, window_height, "GENESIS OS 1.0", aero_color_mode);
        draw_string(window_x + 20, window_y + 60, window_text_line1, 0x000000);
        draw_string(window_x + 20, window_y + 90, window_text_line2, 0x000000);
        draw_string(window_x + 20, window_y + 110, window_text_line3, 0x000000);
    }
    if (file_window_open) draw_file_explorer(file_window_x, file_window_y, file_window_width, file_window_height, aero_color_mode);
    if (notepad_open) draw_notepad(notepad_x, notepad_y, notepad_width, notepad_height, aero_color_mode);
    if (paint_open) draw_paint(paint_x, paint_y, paint_width, paint_height, aero_color_mode);

    draw_cursor(mouse_x, mouse_y);

    for (int i = 0; i < screen_width * screen_height; i++) fb[i] = backbuffer[i];
}

void delay(int count) {
    volatile int i = 0;
    for (i = 0; i < count; i++);
}

void mouse_write(unsigned char data) {
    while ((inb(0x64) & 2) != 0);
    outb(0x64, 0xD4);
    while ((inb(0x64) & 2) != 0);
    outb(0x60, data);
    while ((inb(0x64) & 1) == 0);
    inb(0x60);
}

void init_mouse() {
    while ((inb(0x64) & 2) != 0);
    outb(0x64, 0xA8);
    while ((inb(0x64) & 2) != 0);
    outb(0x64, 0x20);
    while ((inb(0x64) & 1) == 0);
    unsigned char status = inb(0x60);
    status |= 0x02;
    status &= ~0x20;
    while ((inb(0x64) & 2) != 0);
    outb(0x64, 0x60);
    while ((inb(0x64) & 2) != 0);
    outb(0x60, status);
    mouse_write(0xF6);
    mouse_write(0xF4);
}

// ==========================================
//              FUNÇÃO PRINCIPAL
// ==========================================

void kernel_main(multiboot_info_t* mbi) {
    bga_set_mode(1024, 768, 32);
    unsigned int* framebuffer = (unsigned int*)0xFD000000;
    init_mouse();

    // Inicialização do sistema de arquivos
    unsigned short sector_buffer[256] = {0};
    ata_read_sector(1, sector_buffer);
    GfsFileEntry* hd_vfs = (GfsFileEntry*)sector_buffer;

    if (hd_vfs[0].active != 1) {
        vfs[0].active = 1; safe_strcpy(vfs[0].name, "documents", 32); vfs[0].size = 0; vfs[0].is_dir = 1; vfs[0].parent_dir = 0; vfs[0].targets_dir = 1; vfs[0].start_sector = 0;
        vfs[1].active = 1; safe_strcpy(vfs[1].name, "system.txt", 32); vfs[1].size = 31; vfs[1].is_dir = 0; vfs[1].parent_dir = 0; vfs[1].targets_dir = -1; vfs[1].start_sector = 2;
        vfs[2].active = 1; safe_strcpy(vfs[2].name, "..", 32); vfs[2].size = 0; vfs[2].is_dir = 1; vfs[2].parent_dir = 1; vfs[2].targets_dir = 0; vfs[2].start_sector = 0;
        vfs[3].active = 1; safe_strcpy(vfs[3].name, "readme.txt", 32); vfs[3].size = 0; vfs[3].is_dir = 0; vfs[3].parent_dir = 1; vfs[3].targets_dir = -1; vfs[3].start_sector = 3;
        vfs[4].active = 1; safe_strcpy(vfs[4].name, "notes.txt", 32); vfs[4].size = 35; vfs[4].is_dir = 0; vfs[4].parent_dir = 1; vfs[4].targets_dir = -1; vfs[4].start_sector = 4;
        vfs[5].active = 0; safe_strcpy(vfs[5].name, "newfile.txt", 32); vfs[5].size = 12; vfs[5].is_dir = 0; vfs[5].parent_dir = 1; vfs[5].targets_dir = -1; vfs[5].start_sector = 5;

        unsigned char* tar_address = 0;
        if (mbi->flags & (1 << 3)) {
            if (mbi->mods_count > 0) {
                multiboot_module_t* mod = (multiboot_module_t*)mbi->mods_addr;
                tar_address = (unsigned char*)mod->mod_start;
            }
        }
        if (tar_address != 0) {
            int file_size = 0;
            char* file_content = tar_find_file(tar_address, "readme.txt", &file_size);
            if (file_content != 0) {
                vfs[3].size = file_size;
                unsigned short temp_sec[256];
                for (int k = 0; k < 256; k++) temp_sec[k] = 0;
                char* char_temp = (char*)temp_sec;
                for (int k = 0; k < file_size && k < 512; k++) char_temp[k] = file_content[k];
                ata_write_sector(3, temp_sec);
            }
        }
        unsigned short sec2[256]; safe_strcpy((char*)sec2, "GENESIS OS KERNEL V1.0 ACTIVE", 512); ata_write_sector(2, sec2);
        unsigned short sec4[256]; safe_strcpy((char*)sec4, "GENESIS OS HARDWARE IDE DISK SYSTEM", 512); ata_write_sector(4, sec4);
        unsigned short sec5[256]; safe_strcpy((char*)sec5, "NEW ACTIVE FILE", 512); ata_write_sector(5, sec5);

        GfsFileEntry* write_vfs = (GfsFileEntry*)sector_buffer;
        for (int i = 0; i < 6; i++) write_vfs[i] = vfs[i];
        ata_write_sector(1, sector_buffer);

        safe_strcpy(window_text_line1, "DISK INITIALIZED!", 40);
        safe_strcpy(window_text_line2, "VFS FORMATTED TO GFS SYSTEM", 40);
        safe_strcpy(window_text_line3, "PERSISTENCE INSTALLED SUCCESSFULLY", 40);
    } else {
        for (int i = 0; i < 6; i++) vfs[i] = hd_vfs[i];
        safe_strcpy(window_text_line1, "DISK LOADED!", 40);
        safe_strcpy(window_text_line2, "FILES RESTORED FROM DISK.IMG", 40);
        safe_strcpy(window_text_line3, "VFS READ ACTIVE.", 40);
    }

    int mouse_x = 512, mouse_y = 384;
    redraw_screen(framebuffer, screen_width, screen_height, window_open, start_menu_open, mouse_x, mouse_y);

    unsigned char mouse_cycle = 0;
    char mouse_byte[3];
    int last_second = -1;

    // LOOP PRINCIPAL COM REDESENHO CONTÍNUO (~60 FPS)
    while (1) {
        while (inb(0x64) & 1) {
            unsigned char status = inb(0x64);
            unsigned char data = inb(0x60);

            if (status & 0x20) {   // Mouse
                if (mouse_cycle == 0 && !(data & 0x08)) continue;
                mouse_byte[mouse_cycle++] = data;
                if (mouse_cycle == 3) {
                    mouse_cycle = 0;
                    int click_left = mouse_byte[0] & 1;
                    char delta_x = mouse_byte[1];
                    char delta_y = mouse_byte[2];

                    if (delta_x != 0 || delta_y != 0 || click_left != last_click_left) {
                        // Arrasto de janelas/docks (movimento Y corrigido: delta positivo = para cima → Y diminui)
                        if (click_left) {
                            if (!last_click_left) {
                                if (window_open && mouse_x >= window_x && mouse_x < window_x + window_width &&
                                    mouse_y >= window_y && mouse_y < window_y + 28) is_dragging_window = 1;
                                else if (file_window_open && mouse_x >= file_window_x && mouse_x < file_window_x + file_window_width &&
                                         mouse_y >= file_window_y && mouse_y < file_window_y + 28) is_dragging_file_window = 1;
                                else if (notepad_open && mouse_x >= notepad_x && mouse_x < notepad_x + notepad_width &&
                                         mouse_y >= notepad_y && mouse_y < notepad_y + 28) is_dragging_notepad = 1;
                                else if (paint_open && mouse_x >= paint_x && mouse_x < paint_x + paint_width &&
                                         mouse_y >= paint_y && mouse_y < paint_y + 28) is_dragging_paint = 1;
                                else {
                                    for (int i = 0; i < 4; i++) {
                                        if (docks[i].active && mouse_x >= docks[i].x && mouse_x < docks[i].x + docks[i].w &&
                                            mouse_y >= docks[i].y && mouse_y < docks[i].y + 12) {
                                            is_dragging_dock = 1; dragged_dock_index = i; break;
                                        }
                                    }
                                }
                            }
                            // Aplica movimento
                            if (is_dragging_window) { window_x += delta_x; window_y -= delta_y; if (window_y < taskbar_height) window_y = taskbar_height; }
                            else if (is_dragging_file_window) { file_window_x += delta_x; file_window_y -= delta_y; if (file_window_y < taskbar_height) file_window_y = taskbar_height; }
                            else if (is_dragging_notepad) { notepad_x += delta_x; notepad_y -= delta_y; if (notepad_y < taskbar_height) notepad_y = taskbar_height; }
                            else if (is_dragging_paint) { paint_x += delta_x; paint_y -= delta_y; if (paint_y < taskbar_height) paint_y = taskbar_height; }
                            else if (is_dragging_dock && dragged_dock_index != -1) {
                                docks[dragged_dock_index].x += delta_x;
                                docks[dragged_dock_index].y -= delta_y;
                            }
                        } else {
                            is_dragging_window = is_dragging_file_window = is_dragging_notepad = is_dragging_paint = is_dragging_dock = 0;
                            dragged_dock_index = -1;
                        }

                        // Desenho no Paint
                        if (paint_open && click_left && !is_dragging_paint) {
                            int canvas_x = paint_x + 10, canvas_y = paint_y + 36;
                            if (mouse_x >= canvas_x && mouse_x < canvas_x + 128 && mouse_y >= canvas_y && mouse_y < canvas_y + 128) {
                                int lx = (mouse_x - canvas_x) / 2, ly = (mouse_y - canvas_y) / 2;
                                if (lx >= 0 && lx < 64 && ly >= 0 && ly < 64) {
                                    int bit_pos = ly * 64 + lx;
                                    paint_canvas[bit_pos / 8] |= (1 << (bit_pos % 8));
                                }
                            }
                        }

                        // Atualiza posição do mouse
                        mouse_x += delta_x;
                        mouse_y -= delta_y;
                        if (mouse_x < 0) mouse_x = 0;
                        if (mouse_x > screen_width - 12) mouse_x = screen_width - 12;
                        if (mouse_y < 0) mouse_y = 0;
                        if (mouse_y > screen_height - 12) mouse_y = screen_height - 12;

                        // Cliques únicos
                        if (click_left && !last_click_left) {
                            // Fechar janelas
                            if (window_open && mouse_x >= window_x + window_width - 24 && mouse_x < window_x + window_width - 4 &&
                                mouse_y >= window_y + 4 && mouse_y < window_y + 24) { window_open = 0; is_dragging_window = 0; }
                            if (file_window_open && mouse_x >= file_window_x + file_window_width - 24 && mouse_x < file_window_x + file_window_width - 4 &&
                                mouse_y >= file_window_y + 4 && mouse_y < file_window_y + 24) { file_window_open = 0; is_dragging_file_window = 0; }
                            if (notepad_open && mouse_x >= notepad_x + notepad_width - 24 && mouse_x < notepad_x + notepad_width - 4 &&
                                mouse_y >= notepad_y + 4 && mouse_y < notepad_y + 24) { notepad_open = 0; is_dragging_notepad = 0; }
                            if (paint_open && mouse_x >= paint_x + paint_width - 24 && mouse_x < paint_x + paint_width - 4 &&
                                mouse_y >= paint_y + 4 && mouse_y < paint_y + 24) { paint_open = 0; is_dragging_paint = 0; }

                            // Botão SAVE do Notepad
                            if (notepad_open && selected_file_index != -1) {
                                int btn_y = notepad_y + notepad_height - 36;
                                if (mouse_x >= notepad_x + 10 && mouse_x < notepad_x + 100 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    ata_write_sector(vfs[selected_file_index].start_sector, (unsigned short*)notepad_buffer);
                                    vfs[selected_file_index].size = notepad_len;
                                    unsigned short save_sec[256];
                                    GfsFileEntry* hd_write = (GfsFileEntry*)save_sec;
                                    for (int i = 0; i < 6; i++) hd_write[i] = vfs[i];
                                    ata_write_sector(1, save_sec);
                                    safe_strcpy(window_text_line1, "NOTEPAD SAVED!", 40);
                                    safe_strcpy(window_text_line2, "TEXT SAVED PERMANENTLY TO HD", 40);
                                    safe_strcpy(window_text_line3, "", 40);
                                }
                            }
                            // Botão SAVE do Paint
                            if (paint_open && selected_file_index != -1) {
                                int btn_y = paint_y + paint_height - 36;
                                if (mouse_x >= paint_x + 10 && mouse_x < paint_x + 100 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    ata_write_sector(vfs[selected_file_index].start_sector, (unsigned short*)paint_canvas);
                                    safe_strcpy(window_text_line1, "PAINT SAVED!", 40);
                                    safe_strcpy(window_text_line2, "DRAWING PERMANENTLY SAVED TO HD", 40);
                                    safe_strcpy(window_text_line3, "", 40);
                                }
                            }

                            // Botão X do título dos docks (fechar dock)
                            for (int i = 0; i < 4; i++) {
                                if (!docks[i].active) continue;
                                int x_x = docks[i].x + docks[i].w - 16, y_x = docks[i].y + 2;
                                if (mouse_x >= x_x && mouse_x < x_x + 12 && mouse_y >= y_x && mouse_y < y_x + 12) {
                                    docks[i].active = 0;
                                    if (i == 0) start_menu_open = 0;
                                    is_dragging_dock = 0;
                                    goto end_click;
                                }
                                // Clique no título para renomear
                                if (mouse_x >= docks[i].x + 6 && mouse_x < docks[i].x + docks[i].w - 20 &&
                                    mouse_y >= docks[i].y + 2 && mouse_y < docks[i].y + 14) {
                                    rename_dock_active = 1; rename_dock_index = i;
                                    rename_dock_len = 0; rename_dock_buffer[0] = '\0';
                                    safe_strcpy(window_text_line1, "RENAMING DOCK...", 40);
                                    safe_strcpy(window_text_line2, "Type new name, Enter=ok", 40);
                                    safe_strcpy(window_text_line3, "", 40);
                                    goto end_click;
                                }
                            }

                            // Botões internos dos docks
                            if (docks[0].active) {
                                int s_btn_x = docks[0].x + 10, s_btn_y = docks[0].y + 16;
                                if (mouse_x >= s_btn_x && mouse_x < s_btn_x + 100 && mouse_y >= s_btn_y && mouse_y < s_btn_y + 24) {
                                    start_menu_open = !start_menu_open;
                                    if (start_menu_open && start_menu_height == 0) start_menu_height = 1;
                                }
                            }
                            if (docks[1].active) {
                                if (mouse_x >= docks[1].x + 10 && mouse_x < docks[1].x + 85 && mouse_y >= docks[1].y + 16 && mouse_y < docks[1].y + 40) window_open = !window_open;
                                if (mouse_x >= docks[1].x + 95 && mouse_x < docks[1].x + 170 && mouse_y >= docks[1].y + 16 && mouse_y < docks[1].y + 40) file_window_open = !file_window_open;
                                if (mouse_x >= docks[1].x + 180 && mouse_x < docks[1].x + 250 && mouse_y >= docks[1].y + 16 && mouse_y < docks[1].y + 40) aero_color_mode = (aero_color_mode + 1) % 3;
                            }
                            if (docks[2].active) {
                                if (mouse_x >= docks[2].x + 10 && mouse_x < docks[2].x + 100 && mouse_y >= docks[2].y + 16 && mouse_y < docks[2].y + 40)
                                    trigger_shutdown(framebuffer, screen_width, screen_height);
                            }

                            // Menu iniciar (só aceita cliques quando está totalmente expandido)
                            if (start_menu_open && start_menu_height >= 160) {
                                int menu_x = docks[0].x;
                                int menu_y = docks[0].y - start_menu_height;
                                if (menu_y < 0) menu_y = 0;
                                if (mouse_x >= menu_x && mouse_x < menu_x + 160) {
                                    if (mouse_y >= menu_y + 15 && mouse_y < menu_y + 40) { window_open = 1; safe_strcpy(window_text_line1, "GENESIS OS V1.0", 40); safe_strcpy(window_text_line2, "MEM. 128MB  RES. 1024X768", 40); safe_strcpy(window_text_line3, "32-BIT KERNEL ACTIVE", 40); start_menu_open = 0; }
                                    else if (mouse_y >= menu_y + 45 && mouse_y < menu_y + 70) { notepad_open = 1; paint_open = 0; start_menu_open = 0; }
                                    else if (mouse_y >= menu_y + 75 && mouse_y < menu_y + 100) { paint_open = 1; notepad_open = 0; start_menu_open = 0; }
                                    else if (mouse_y >= menu_y + 105 && mouse_y < menu_y + 130) { aero_color_mode = (aero_color_mode + 1) % 3; start_menu_open = 0; }
                                    else if (mouse_y >= menu_y + 135 && mouse_y < menu_y + 160) trigger_shutdown(framebuffer, screen_width, screen_height);
                                }
                            }

                            // File Explorer (abrir, info, deletar, renomear, novo)
                            if (file_window_open) {
                                int list_x = file_window_x + 10, list_y = file_window_y + 64;
                                int visible_files[6], visible_count = 0;
                                for (int v = 0; v < 6; v++) if (vfs[v].active && vfs[v].parent_dir == current_dir) visible_files[visible_count++] = v;
                                if (mouse_x >= list_x && mouse_x < list_x + file_window_width - 20 && mouse_y >= list_y + 4 && mouse_y < list_y + 150) {
                                    int clicked_row = (mouse_y - (list_y + 8)) / 20;
                                    if (clicked_row >= 0 && clicked_row < visible_count) { selected_file_index = visible_files[clicked_row]; rename_active = 0; }
                                }
                                int btn_y = file_window_y + 240;
                                if (mouse_x >= file_window_x + 10 && mouse_x < file_window_x + 100 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    if (selected_file_index != -1 && vfs[selected_file_index].active) {
                                        if (vfs[selected_file_index].is_dir) { current_dir = vfs[selected_file_index].targets_dir; selected_file_index = -1; }
                                        else {
                                            window_open = 1; safe_strcpy(window_text_line1, "", 40); safe_strcpy(window_text_line2, "", 40); safe_strcpy(window_text_line3, "", 40);
                                            unsigned short read_sec[256]; ata_read_sector(vfs[selected_file_index].start_sector, read_sec);
                                            const char* file_content = (const char*)read_sec; int file_size = vfs[selected_file_index].size; if (file_size > 511) file_size = 511;
                                            int line = 1, idx = 0;
                                            for (int k = 0; k < file_size; k++) {
                                                char ch = file_content[k];
                                                if (ch == '\n' || ch == '\r' || ch == '\0') { if (line == 1) window_text_line1[idx] = '\0'; else if (line == 2) window_text_line2[idx] = '\0'; else if (line == 3) window_text_line3[idx] = '\0'; line++; idx = 0; if (line > 3) break; }
                                                else { if (idx < 39) { if (line == 1) window_text_line1[idx++] = ch; else if (line == 2) window_text_line2[idx++] = ch; else if (line == 3) window_text_line3[idx++] = ch; } }
                                            }
                                        }
                                    }
                                }
                                if (mouse_x >= file_window_x + 110 && mouse_x < file_window_x + 200 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    if (selected_file_index != -1 && vfs[selected_file_index].active) {
                                        window_open = 1;
                                        safe_strcpy(window_text_line1, "NAME. ", 40); safe_strcpy(window_text_line2, "TYPE. ", 40); safe_strcpy(window_text_line3, "SIZE. 0 BYTES", 40);
                                        int len = 6; for (int n = 0; vfs[selected_file_index].name[n] && len < 39; n++) window_text_line1[len++] = vfs[selected_file_index].name[n]; window_text_line1[len] = '\0';
                                        len = 6; const char* t = vfs[selected_file_index].is_dir ? "DIRECTORY" : "TEXT FILE"; for (int n = 0; t[n] && len < 39; n++) window_text_line2[len++] = t[n]; window_text_line2[len] = '\0';
                                        if (vfs[selected_file_index].size > 0) safe_strcpy(window_text_line3, "SIZE. ACTIVE FILE", 40);
                                    }
                                }
                                if (mouse_x >= file_window_x + 210 && mouse_x < file_window_x + 300 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    if (selected_file_index != -1 && vfs[selected_file_index].active && strcmp(vfs[selected_file_index].name, "..") != 0) {
                                        vfs[selected_file_index].active = 0;
                                        unsigned short save_sec[256]; GfsFileEntry* hd_write = (GfsFileEntry*)save_sec; for (int i = 0; i < 6; i++) hd_write[i] = vfs[i]; ata_write_sector(1, save_sec);
                                        selected_file_index = -1;
                                    }
                                }
                                if (mouse_x >= file_window_x + 265 && mouse_x < file_window_x + 340 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    if (selected_file_index != -1 && vfs[selected_file_index].active && strcmp(vfs[selected_file_index].name, "..") != 0) {
                                        rename_active = 1; rename_len = 0; rename_buffer[0] = '\0';
                                        safe_strcpy(window_text_line1, "RENAME MODE ACTIVE", 40); safe_strcpy(window_text_line2, "TYPE THE NEW FILE NAME", 40); safe_strcpy(window_text_line3, "PRESS ENTER TO CONFIRM RENAME", 40);
                                    }
                                }
                                if (mouse_x >= file_window_x + 350 && mouse_x < file_window_x + 430 && mouse_y >= btn_y && mouse_y < btn_y + 24) {
                                    vfs[5].active = 1; vfs[5].parent_dir = current_dir;
                                    unsigned short save_sec[256]; GfsFileEntry* hd_write = (GfsFileEntry*)save_sec; for (int i = 0; i < 6; i++) hd_write[i] = vfs[i]; ata_write_sector(1, save_sec);
                                    selected_file_index = 5;
                                }
                            }
                            end_click:;
                        }
                        redraw_screen(framebuffer, screen_width, screen_height, window_open, start_menu_open, mouse_x, mouse_y);
                        last_click_left = click_left;
                    }
                }
            } else {   // Teclado
                if (!(data & 0x80)) {
                    char ascii = keyboard_map[data];
                    if (ascii != 0) {
                        // Renomeação de dock
                        if (rename_dock_active && rename_dock_index >= 0) {
                            if (ascii == '\n') {
                                rename_dock_buffer[rename_dock_len] = '\0';
                                safe_strcpy((char*)docks[rename_dock_index].name, rename_dock_buffer, 100);
                                rename_dock_active = 0; rename_dock_index = -1;
                            } else if (ascii == '\b') {
                                if (rename_dock_len > 0) rename_dock_len--;
                                rename_dock_buffer[rename_dock_len] = '\0';
                            } else if (rename_dock_len < 31) {
                                rename_dock_buffer[rename_dock_len++] = ascii;
                                rename_dock_buffer[rename_dock_len] = '\0';
                            }
                            continue;
                        }
                        // Renomeação de arquivo
                        if (rename_active && selected_file_index != -1) {
                            if (ascii == '\n') {
                                rename_buffer[rename_len] = '\0';
                                safe_strcpy(vfs[selected_file_index].name, rename_buffer, 32);
                                unsigned short save_sec[256]; GfsFileEntry* hd_write = (GfsFileEntry*)save_sec; for (int i = 0; i < 6; i++) hd_write[i] = vfs[i]; ata_write_sector(1, save_sec);
                                rename_active = 0;
                                safe_strcpy(window_text_line1, "FILE RENAMED SUCCESS!", 40); safe_strcpy(window_text_line2, "NAME SAVED TO HD SECTOR 1", 40); safe_strcpy(window_text_line3, "", 40);
                            } else if (ascii == '\b') { if (rename_len > 0) rename_len--; rename_buffer[rename_len] = '\0'; }
                            else if (rename_len < 30) { rename_buffer[rename_len++] = ascii; rename_buffer[rename_len] = '\0'; }
                        }
                        // Entrada no Notepad
                        else if (notepad_open) {
                            if (ascii == '\b') { if (notepad_len > 0) notepad_len--; notepad_buffer[notepad_len] = '\0'; }
                            else if (ascii == '\n') { if (notepad_len < 510) { notepad_buffer[notepad_len++] = '\n'; notepad_buffer[notepad_len] = '\0'; } }
                            else { if (notepad_len < 511) { notepad_buffer[notepad_len++] = ascii; notepad_buffer[notepad_len] = '\0'; } }
                        }
                    }
                }
            }
        }
        // Redesenho contínuo para animações (~60 FPS)
        redraw_screen(framebuffer, screen_width, screen_height, window_open, start_menu_open, mouse_x, mouse_y);
        delay(8000);   // ajuste para ~60 FPS
    }
}
