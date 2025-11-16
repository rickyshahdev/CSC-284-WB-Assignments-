#!/bin/bash
echo "Setting up User Profile Updater..."

# Remove any old files and start fresh
rm -f user_profile_updater
rm -f *.o

# Compile the program
echo "Compiling..."
clang++ -std=c++17 -Wall -Wextra main.cpp json_processor.cpp file_utils.cpp -o user_profile_updater

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    
    # Create test directory structure
    echo "Creating test directory structure..."
    mkdir -p user_profiles/admins
    mkdir -p user_profiles/editors
    mkdir -p user_profiles/viewers
    
    # Create sample admin file
    cat > user_profiles/admins/admin1.json << 'EOF'
{
    "username": "admin1",
    "email": "admin1@company.com",
    "status": "enabled",
    "permissions": ["manage_users", "view_content"],
    "profile": {
        "name": "Admin User",
        "contact": "admin1@company.com"
    }
}
EOF

    # Create sample editor file
    cat > user_profiles/editors/editor1.json << 'EOF'
{
    "username": "editor1",
    "email": "editor1@company.com",
    "status": "enabled",
    "permissions": ["view_content"],
    "profile": {
        "name": "Editor User",
        "contact": "editor1@company.com"
    }
}
EOF

    echo "Test data created!"
    echo ""
    echo "To run the program: ./user_profile_updater user_profiles"
else
    echo "Compilation failed!"
    exit 1
fi