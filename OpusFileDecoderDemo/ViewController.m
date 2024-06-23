//
//  ViewController.m
//  OpusFileDecoderDemo
//
//  Created by Drops on 2024/6/23.
//

#import "ViewController.h"
#import "OpusDecoder/OpusFileDecoder.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    NSString *nsopusPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject stringByAppendingPathComponent:@"1.opus"];
    NSString *wavPath = [nsopusPath stringByAppendingString:@".wav"];
    int ret = opus2wav(nsopusPath.UTF8String, wavPath.UTF8String);
    NSLog(@"ret:%d %@", ret, wavPath);
}


@end
