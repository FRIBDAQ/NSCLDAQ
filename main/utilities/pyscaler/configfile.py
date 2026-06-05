#!/usr/bin/env python3

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321

'''
    This file provides the code to read scaler display
    configuration files.  See the 'design-musings.txt' file
    and later in the project, the  documentation for the form
    of these files.... other than that TOML is used for the
    general form.
    
    @file configfile.py
    @brief Read scaler configuration files.
    @author Ron Fox
    @note Installed to be imported as import ScalerDisplay.configfile e.g.
'''

import tomllib
import fnmatch

DEFAULT_VERSION=12.0    # Default DAQ version.

class Configuration:
    '''
    This class parses and contains
    a parsed configuration along with methods to 
    support pulling appropriate bits out of the
    configuration
    '''
    default_alarm_colors = {
        'lowalarm' : 'green', 'highalarm' : 'red', 'noalarm' :'white', 'bothalarms' : 'yellow'
    }
    def __init__(self, toml):
        '''
            Construct the configuration from a 
            string of toml.
            
            @param toml - string containing toml
            @note we will let the parsing exceptions pass back
            to the caller for them to handle.
        '''
        self._rawconfig = tomllib.loads(toml)  # Can throw.
        
    def datasources(self):
        ''' 
            Returns an array of data sources.  Each element of the array
            is a dict with the keys:
            *  name - name of the data source.
            *  url  - URI of the ring buffer that is the data source.
            *  version - DAQ version.
            *  scalers - array of scalers that are contributed by that source.
            
         @note and empty array as a result indicates the user does not define and datasources.
        '''
        result = []
        if 'datasource' in self._rawconfig.keys():
            datasources = self._rawconfig['datasource']
            for key, contents in datasources.items():
                source_dict = {
                    'name' : key, 'url': contents['url'], 'scalers' : contents['scalers']
                }
                if 'version' in contents.keys() :
                    source_dict['version'] = contents['version']
                else :
                    source_dict['version'] = DEFAULT_VERSION
                    
                result.append(source_dict)

        return result
    
    def pages(self):
        '''
            Returns information about the counter pages in the configuration.
            This is returned as an array of dicts with the keys:
            * name - name of the page.
            * title - Page title
            * lines - an array of line specification to display.  Each line specification is
                      a dict containing:
                      number - the line number
                      type   - the line type, one of 'single', 'pair' or 'ratio'
                      scalers - an array of scaler specifications in the form source_name.scaler_name.
        '''
        result =[]

        if 'pages' in self._rawconfig.keys():
            for key, contents in self._rawconfig['pages'].items():
                page_dict = {
                    'name': key, 'title' : contents['title'],
                    'lines' : []
                }
                # Figure out the lines array:
                
                for key, value in contents.items():
                    if fnmatch.fnmatch(key, 'line*'):
                        digits = "".join(filter(str.isdigit, key))
                        if digits:
                            number = int(digits)
                            line_dict = {
                                'number': number, 'type': value['type'], 
                                'scalers': value['scalers']
                            }
                            page_dict['lines'].append(line_dict)
                        
                result.append(page_dict)
        return result
    
    def plots(self):
        '''
            Returns information about what the user wants in the optional strip chart.
            This will be a dict with the keys:
            * single - an array of scaler names whose rates will be strip charted.
            * ratio - an array of scaler pairs whos ratio will be strip charted [numerator, denominator]
            
            It is possible for either or both to be empty.
        '''
        result = {'single' : [], 'ratio' :[]}
        if 'graph' in self._rawconfig.keys():
            if 'individual' in self._rawconfig['graph'].keys():
                result['single'] = self._rawconfig['graph']['individual']
            if 'ratios' in self._rawconfig['graph'].keys():
                result['ratio'] = self._rawconfig['graph']['ratios']
        return result 
        
    def alarms(self):
        '''
        Returns information about which scalers should be alarmed.
        This is a (possibly empty) dict with keys the full scaler names (e.g. source_name.scaler_name) where
        the contents of each key is a dict with they keys:
        'low' - the low alarm threshold or None if there isn't one.
        'high - the high alarm threshold or None if there isn't one.
        
        @note that in a properly formed config file at least one of those keys will be present.
        @note if a scaler appears (by mistake most likely), the last of the occurences rules.
        
        '''
        result = {}
        print("computing alarms")
        if 'alarms' in self._rawconfig.keys() :
            print(self._rawconfig['alarms'])
            for src_name, scalers in self._rawconfig['alarms'].items():
                for scaler_name, alarm_spec in scalers.items():
                    print(scaler_name, alarm_spec)
                    fullname='.'.join([src_name, scaler_name])
                    result[fullname] = {'low': None, 'high' : None}
                    if 'low' in alarm_spec.keys():
                        result[fullname]['low'] = alarm_spec['low']
                    if 'high' in alarm_spec.keys():
                        result[fullname]['high'] = alarm_spec['high']
        
        return result
        
    def alarm_colors(self):
        
        '''
            Returns a dict with the alarm colors.  These are used:
            - For the scaler names that are in alarmed conditions.
            - For the tab names that have alarms.
            
            The result is a dict containing the keys:
            
            'lowalarm' - color to use when scalers are below the low alarm threshold.
            'highalarm' - color to use when scalers are above the high alarm threshold.
            'noalarm'   - color for tab text when there are no alarms on the page.
            'bothalamrs' - Color to use for the tab text when a page has both low and high alarms.
            
            @note this dict is always fully populated with at least the default values.
        '''
        result = self.default_alarm_colors     # Start with the defaults...
        
        # Override  the defaults with any  alarm colors provided:
        
        if 'alarmcolors' in self._rawconfig.keys() : 
            colors = self._rawconfig['alarmcolors']
            
            if 'lowalarm' in colors.keys():
                result['lowalarm'] = colors['lowalarm']
            if 'highalarm' in colors.keys():
                result['highalarm'] = colors['highalarm']
            if 'bothalarms' in colors.keys():
                result['bothalarms'] = colors['bothalarms']
            if 'noalarm' in colors.keys() :
                result['noalarm'] = colors['noalarm']
        return result
    def output_path(self) -> str:
        '''
            @return str - the path where the end run summary files should
                          go. 
            @retval '." if the toml does not specify it.
            
        '''
        if 'output' in self._rawconfig:
            output = self._rawconfig['output']
            if 'path' in output:
                return output['path']
        return '.'                   # Default value.
        
    def check(self):
        '''
            Performs some checking on the semantics of the
            configuration that was read in.
            0.  There must bge at least one data source.
            1.  data sources must have a url and a
                scaler key
            2.  Pages must have a title and at least one line*
                element.
            3.  The lines must have a type and a scalers key.
                the type must have a valid value.
            4.  all of the scalers in the lines must have been
                defined by the data sources.
            5.  If a graph is present, the scalers in the
                optional individual and ratio keys must be defined.
        
        @return an array of error messages (hopefully empty):
        @note we don't assume that any of the getter functions will
        properly work.
        
        '''
        # sorry, this is a rather long, but reltatively straightforward
        #      method
    
        lines = []
        scaler_names = []
        
        # Validate the data sources and accumulate the  list of full scaler names.
        
        if 'datasource' in self._rawconfig.keys():
            ds = self._rawconfig['datasource']
            if len(ds) > 0:
                # accumulate the scaler definitions and ensure each data source definition
                # has a URL and scalers key:
                
                for key, definition in ds.items():
                    if 'url' not in definition.keys():
                        lines.append(f'The definition of data source {key} is missing a "url" key.')
                    else:
                        if 'scalers' not in definition.keys():
                            lines.append(f'The definition of data source {key} has no "scalers" list')
                        else:
                            # Collect the scalers with their fullnames:
                            for name in definition['scalers']:
                                full_name = f'{key}.{name}'
                                if full_name in scaler_names:
                                    lines.append(f'The definition of data sourcde {key} has a  duplicat scaler {full_name}')
                                scaler_names.append(full_name)
            else:
                lines.append("you must have at least one data source specificaton [datasource.spdaq11] e.g.")
        else:
            lines.append("you must have at least one data source specificaton [datasource.spdaq11] e.g.")
            
        # validate the pages and their lines:
        
        if 'pages' in self._rawconfig.keys():
            pages = self._rawconfig['pages']
            if len(pages) > 0:
                for name, definition in pages.items():
                    if 'title' not in definition.keys():
                        lines.append(f'Page {name} is missing a title')
                    line_count = 0
                    for key, line in definition.items():
                        if fnmatch.fnmatch(key, 'line*'):
                            digits = "".join(filter(str.isdigit, key))
                            if digits:
                                line_count += 1
                                if 'type' in line:
                                    if line['type'] not in ['single', 'pair', 'ratio']:
                                        lines.append(f'line {key} in {name} invalid type {definition["type"]}')
                                else:
                                    lines.append(f'line {key} in {name} is missing a "type" specification.')
                                if 'scalers' in line:
                                    scalers = line['scalers']
                                    for s in scalers:
                                        if s not in scaler_names:
                                            lines.append(f'{s} is not a known scaler in page {name} line {key}')
                                else:
                                    lines.append(f'There is no "scaler" key in page {name} line{key}')
                            else:
                                lines.append(f'Line {key} in {name} is not a valid line.number string.')
                    if line_count == 0:
                        lines.append(f'page {name} does not have any line definitions.')    
            else:
                lines.append('There are no page definitions in the configuration file.')
        else:
            lines.append('There are no page definitions in the configuration file.')
            
        # graph is totally optional but if it is used, the scalers must be defined:
        
        if 'graph' in self._rawconfig.keys():
            graph = self._rawconfig['graph']
            
            # Check any individual ones:
            if 'individual' in graph.keys():
                scalers = graph['individual']
                for s in scalers:
                    if s not in scaler_names:
                        lines.append(f'[graph] individual scaler {s} is not defined.')
        
            if 'ratios' in graph.keys():
                ratios = graph['ratios']     # Note array elements must be pairs:
                for ratio in ratios:
                    if len(ratio) == 2:
                        if ratio[0] not in scaler_names:
                            lines.append(f'[ratio] {ratio} scaler {ratio[0]} is not defined')
                        if ratio[1] not in scaler_names:
                            lines.append(f'[ratio] {ratio} scaler {ratio[1]} is not defined')
                    else:
                        lines.append(f'[ratio] {ratio}  must be a 2 element array and is not')
                        
            # Check the alamrs, keys must be valid scalers
            
            if 'alarms' in self._rawconfig.keys():
                alarms = self._rawconfig['alarms']
                for src, item in alarms.items():
                    for scaler in item:
                        full_name = f'{src}.{scaler}'
                    if full_name not in scaler_names:
                        lines.append(f'Alarm definition for {key} is not a defined scaler')
        return lines
        
    
## If we are the main we can run tests:

if __name__ == '__main__':
    import unittest

    class Tests(unittest.TestCase):
        #  This is the toml we'll test.
        test_toml = '''
[datasource]

[datasource.raw1]
url='tcp://spdaq10.frib.msu.edu/raw_1'
scalers=['name1', 'name2', 'name3']

[datasource.raw2]
url='tcp://spdaq11.frib.msu.edu'
version=11.0
scalers=['name1', 'name2', 'name3']


[pages]

[pages.apage]
title='This is the page title'
line1= {type='single', scalers=['raw2.name1']}    # Single scaler from raw1 source.
line2= {type = 'pair', scalers=['raw1.name1', 'raw1.name2']} # Pair of scalers no ratio done.
line3= {type = 'ratio', scalers=['raw2.name2', 'raw2.name3']} #  Pair with ratios.

[graph]
individual=['raw1.name1', 'raw2.name2']  # Graphed as individual scalers
ratios = [['raw1.name2', 'raw1.name3'], ['raw2.name1', 'raw2.name2']]  # Graphed as ratios.


[alarms]
raw1.name1={high=1234, low=10}  # omitted entries don't have that alarm type.

[alarmcolors]
lowalarm='blue'
highalarm='yellow'
bothalarms='green'
noalarm='black'

[output]
path="/user/ron/scaler_info"
            '''
            
        def setUp(self) :
            return Configuration(self.test_toml)
        
        def test_source(self):
            config = self.setUp()
            sources = config.datasources()
            self.assertEqual(2, len(sources))
            
            src0 = sources[0]
            self.assertEqual('tcp://spdaq10.frib.msu.edu/raw_1', src0['url'])
            self.assertEqual(['name1', 'name2', 'name3'], src0['scalers'])
            self.assertEqual(DEFAULT_VERSION, src0['version'])
            
            src1 = sources[1]
            self.assertEqual('tcp://spdaq11.frib.msu.edu', src1['url'])
            self.assertEqual(['name1', 'name2', 'name3'], src1['scalers'])
            self.assertEqual(11.0, src1['version'])
    
        def test_pages(self):
            config = self.setUp()
            
            pages = config.pages()
            self.assertEqual(1, len(pages))
            page = pages[0]
            self.assertEqual('This is the page title', page['title'])
            self.assertEqual('apage', page['name'])
            lines = page['lines']
            self.assertEqual(3, len(lines))
            
            self.assertEqual(1, lines[0]['number'])
            self.assertEqual('single', lines[0]['type'])
            self.assertEqual(['raw2.name1'], lines[0]['scalers'])
            
            self.assertEqual(2, lines[1]['number'])
            self.assertEqual('pair', lines[1]['type'])
            self.assertEqual(['raw1.name1', 'raw1.name2'], lines[1]['scalers'])
            
            self.assertEqual(3, lines[2]['number'])
            self.assertEqual('ratio', lines[2]['type'])
            self.assertEqual(['raw2.name2', 'raw2.name3'], lines[2]['scalers'])
        
        def test_plots(self):
            config = self.setUp()
            plots = config.plots()
            self.assertTrue('single' in plots.keys())
            self.assertEqual(['raw1.name1', 'raw2.name2'], plots['single'])
            
            self.assertTrue('ratio' in plots.keys())
            self.assertEqual(
                [['raw1.name2', 'raw1.name3'], ['raw2.name1', 'raw2.name2']],
                plots['ratio']
            )
            
        def test_alarms(self):
            config = self.setUp()
            alarms = config.alarms()
            self.assertTrue('raw1.name1' in alarms.keys())
            
            self.assertTrue('low' in alarms['raw1.name1'].keys())
            self.assertEqual(10, alarms['raw1.name1']['low'])
            
            self.assertTrue('high' in alarms['raw1.name1'].keys())
            self.assertEqual(1234, alarms['raw1.name1']['high'])
            
        def test_alarm_colors(self):
            config = self.setUp()
            colors = config.alarm_colors()
        
            self.assertTrue('lowalarm' in colors.keys())
            self.assertEqual('blue', colors['lowalarm'])
            
            self.assertTrue('highalarm' in colors.keys())
            self.assertEqual('yellow', colors['highalarm'])
            
            self.assertTrue('bothalarms' in colors.keys())
            self.assertEqual('green', colors['bothalarms'])
            
            self.assertTrue('noalarm' in colors.keys())
            self.assertEqual('black', colors['noalarm'])
            
        def test_check(self):
            config = self.setUp()
            lines = config.check()
            self.assertEqual(0, len(lines))
        def test_output(self):
            config = self.setUp()    
            self.assertEqual("/user/ron/scaler_info", config.output_path())
    unittest.main()
        