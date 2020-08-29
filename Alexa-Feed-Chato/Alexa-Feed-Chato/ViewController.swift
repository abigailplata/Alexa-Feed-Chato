//
//  ViewController.swift
//  Alexa-Feed-Chato
//
//  Created by Abigail Plata on 8/28/20.
//  Copyright © 2020 Abigail Plata. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    @IBOutlet weak var FeedSizeNum: UILabel!
    @IBOutlet weak var FeedIntervalNum: UILabel!
    
    @IBAction func FeedSize(_ sender: UISlider){
        FeedSizeNum.text = String( Int(roundf(sender.value / 5.0) * 5.0))
    }
    
    @IBAction func FeedInterval(_ sender: UISlider){
        FeedIntervalNum.text = String( Int(roundf(sender.value / 2.0) * 2.0))
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view, typically from a nib.
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }


}

