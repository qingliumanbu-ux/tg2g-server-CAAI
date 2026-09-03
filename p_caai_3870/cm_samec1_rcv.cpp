/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: 物料编码接收信息
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 
BM2F_ENTERACE_TELE(cm_samec1_rcv)

int f_cm_samec1_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaac11("TCAAC11");
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//Log::Trace("", __FUNCTION__, "tcaac11[MAT_TYPE]=[{0}]  ", bcls_rec->Tables[0].Rows[i]["MTART"].ToString().TrimOrBlank());
			tcaac11.Reset();

			//by20201029 为了有些配置在MES上已经设置了
			tcaac11["MAT_CODE"] = bcls_rec->Tables[0].Rows[i]["MATNR"].ToString().TrimOrBlank();
			tcaac11.Query("MAT_CODE");
			Log::Trace("", __FUNCTION__, "mat_TYPE=[{0}]  ", tcaac11["MAT_TYPE"].ToString());
			//tcaac11.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tcaac11["MAT_TYPE_CODE"] = bcls_rec->Tables[0].Rows[i]["MTART"].ToString().TrimOrBlank();
			tcaac11["TYPE_DESC"] = bcls_rec->Tables[0].Rows[i]["MTART_DES"].ToString().TrimOrBlank();			
			tcaac11["MAT_NAME"] = bcls_rec->Tables[0].Rows[i]["MAKTX"].ToString().TrimOrBlank();
			tcaac11["UNIT"] = bcls_rec->Tables[0].Rows[i]["MEINS"].ToString().TrimOrBlank();
			tcaac11["UNIT_DESC"] = bcls_rec->Tables[0].Rows[i]["MEINS_DES"].ToString().TrimOrBlank();
			tcaac11["USE_FLAG_1"] = bcls_rec->Tables[0].Rows[i]["LVORM"].ToString().TrimOrBlank();
			tcaac11["CHECK_CONFM_FLAG"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION1"].ToString().TrimOrBlank();
			tcaac11["REMARK_2"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION2"].ToString().TrimOrBlank();
			tcaac11["REMARK_3"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION3"].ToString().TrimOrBlank();
			tcaac11["REMARK_4"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION4"].ToString().TrimOrBlank();
			tcaac11["REMARK_5"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION5"].ToString().TrimOrBlank();
			tcaac11["REMARK_6"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION6"].ToString().TrimOrBlank();
			tcaac11["REMARK_7"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION7"].ToString().TrimOrBlank();
			tcaac11["REMARK_8"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION8"].ToString().TrimOrBlank();
			tcaac11["REMARK_11"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION9"].ToString().TrimOrBlank();
			tcaac11["REMARK_10"] = bcls_rec->Tables[0].Rows[i]["ZEXTENSION10"].ToString().TrimOrBlank();
			Log::Trace("", __FUNCTION__, "MAT_CODE=[{0}]", bcls_rec->Tables[0].Rows[i]["MATNR"].ToString().TrimOrBlank());

			

			tcaac11["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tcaac11["REC_CREATOR"] = s.userid;
			tcaac11.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0
			tcaac11.Delete("MAT_CODE");
			tcaac11.Insert();
			//

			//if (tcaac11["MAT_CODE"].ToString().TrimOrBlank() != "")
			if (tcaac11["MAT_CODE"].ToString().TrimOrBlank() == bcls_rec->Tables[1].Rows[i]["MATNR"].ToString().TrimOrBlank())
			{
				//tcaac11.MergeFrom(bcls_rec->Tables[1].Rows[i]);
				tcaac11["DEPT_CODE"] = bcls_rec->Tables[1].Rows[i]["WERKS"].ToString().TrimOrBlank();
				tcaac11["PROD_CODE"] = bcls_rec->Tables[1].Rows[i]["ZPINMINGDAIMA"].ToString().TrimOrBlank();
				tcaac11["PROD_CNAME"] = bcls_rec->Tables[1].Rows[i]["PROD_CNAME"].ToString().TrimOrBlank();
				tcaac11["SG_CODE"] = bcls_rec->Tables[1].Rows[i]["ZPAIHAOSE"].ToString().TrimOrBlank();
				tcaac11["SG_SIGN"] = bcls_rec->Tables[1].Rows[i]["ZPAIHAOSE_DES"].ToString().TrimOrBlank();
				tcaac11["STD_CODE"] = bcls_rec->Tables[1].Rows[i]["ZBIAOZHUNDAIMA"].ToString().TrimOrBlank();
				tcaac11["STD_NAME"] = bcls_rec->Tables[1].Rows[i]["STD_NAME"].ToString().TrimOrBlank();
				tcaac11["SPEC"] = bcls_rec->Tables[1].Rows[i]["ZGUIGEZUJU"].ToString().TrimOrBlank();
				tcaac11["REMARK_SPECS"] = bcls_rec->Tables[1].Rows[i]["ZGUIGEZUJU_DES"].ToString().TrimOrBlank();
				tcaac11["DELIVY_STATUS_CODE"] = bcls_rec->Tables[1].Rows[i]["DELIVY_STATUS_NO"].ToString().TrimOrBlank();
				tcaac11["DELIVY_STATUS"] = bcls_rec->Tables[1].Rows[i]["DELIVY_STATUS"].ToString().TrimOrBlank();
				tcaac11["HEAT_TREAT_METHOD_CODE"] = bcls_rec->Tables[1].Rows[i]["ZRECHULIFANGSHI"].ToString().TrimOrBlank();
				tcaac11["HEAT_TREAT_METHOD_DESC"] = bcls_rec->Tables[1].Rows[i]["HEAT_TREAT_DESC"].ToString().TrimOrBlank();
				tcaac11["STATUS"] = bcls_rec->Tables[1].Rows[i]["ZGONGXUZHUANGTAI"].ToString().TrimOrBlank();
				tcaac11["STATUS_DESC"] = bcls_rec->Tables[1].Rows[i]["STATUS_DESC"].ToString().TrimOrBlank();
				Log::Trace("", __FUNCTION__, "mat_code=[{0}]  ",tcaac11["MAT_CODE"].ToString().TrimOrBlank());
				tcaac11.Update("*","MAT_CODE");
			}
		}

	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


