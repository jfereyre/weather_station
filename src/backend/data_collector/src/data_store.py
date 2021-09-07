import json
import os.path

'''
'''
class DataStore:
    '''
    '''
    class __DataStore:
        '''
        '''
        def __init__(self):
            self.__m_file_name = None
            self.__m_data_buffer = {}
            self.__buffering_threshold = 1

        '''
        '''
        def __str__(self):
            return repr(self)
    
        '''
        '''
        def __init_file(self):

            l_initial_data = {}

            # Check that the data file exists
            # if not initialize it.
            if not os.path.isfile(self.__m_file_name):
                with open(self.__m_file_name,'w+') as l_file:
                    json.dump(l_initial_data, l_file , indent = 4)

        '''
        '''
        def setFile(self, a_file_name):
            self.__m_file_name = a_file_name

            self.__init_file()

        '''
        '''
        def store(self, a_json, a_category="datas"):
            self.__m_data_buffer.setdefault(a_category, []).append(a_json)

            if len(self.__m_data_buffer) >= self.__buffering_threshold and self.__m_file_name:
                self.flushToFile()

        '''
        '''
        def flushToFile(self):
            if not self.__m_file_name:
                print("File not specified. Flush aborted.\n")
                return

            with open(self.__m_file_name,'r+') as l_file:

                # First we load existing data into a dict.
                l_file_data = json.load(l_file)
                
                for l_category in self.__m_data_buffer:
                    try:
                        l_new_data = self.__m_data_buffer[l_category].pop()

                        while l_new_data:
                            # Join l_new_data with l_file_data inside datas
                            l_file_data.setdefault(l_category, []).append(l_new_data)

                            l_new_data = self.__m_data_buffer[l_category].pop()
                    except IndexError:
                        pass

                # Sets l_file's current position at offset 0
                l_file.seek(0)

                # Convert back to json.
                json.dump(l_file_data, l_file, indent = 4)

        '''
        '''
        def getCategoryData(self, a_category="datas"):
            if not self.__m_file_name:
                print("File not specified. Data retrieval aborted.\n")
                return

            l_datas = None

            with open(self.__m_file_name,'r+') as l_file:

                # First we load existing data into a dict.
                l_file_data = json.load(l_file)
                
                l_datas = l_file_data.get(a_category, [])

                if a_category in self.__m_data_buffer:
                    l_datas.extend(self.__m_data_buffer[a_category])

            return l_datas

    instance = None

    '''
    '''
    def __init__(self, a_file_name):
        if not DataStore.instance:
            DataStore.instance = DataStore.__DataStore()
            
        DataStore.instance.setFile(a_file_name)
        
    '''
    '''
    def __getattr__(self, name):
        return getattr(self.instance, name)


def main():
    d = DataStore("log.json")

    print(json.dumps(d.getCategoryData("rain_gauge")))

if __name__ == '__main__':
    main()