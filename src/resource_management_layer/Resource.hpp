#pragma once 
#include <string>
class Resource{

    public:
        explicit Resource(const std::string& id) : resourceId(id){}
        //Destructor de la clase
        virtual ~Resource() = default;

        //Identidad de recursos y metodos de acceso de estado
        const std::string& GetId() const {return resourceId;}
        bool IsLoaded() const {return loaded;}

        //Interfaz virtual para el comportamiento de la carga y decarga de un recurso en especifico
        bool Load(){
            loaded = doLoad();
            return loaded;
        }

        void Unload(){
            doUnload();
            loaded = false;
        }
    
    protected:
        //Para el patron de diseño template que se usara en el codigo
        virtual bool doLoad() = 0;
        virtual bool doUnload() = 0;

    private:
        std::string resourceId;
        bool loaded = false;


};